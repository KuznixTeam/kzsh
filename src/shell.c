/*
 * Portable shell front-end (no readline)
 *
 * Prompt format:
 *   [{username}@{hostname} {folder}] $
 *
 * Folder rules:
 *  - If cwd equals user's home and NOT running as root -> "~"
 *  - Otherwise show basename(cwd) (for root this will show e.g. "root" for /root,
 *    but the default behavior is to not abbreviate the home path when running as root)
 *
 * PS1 escapes supported (in expand_ps1):
 *   \u  -> username
 *   \h  -> hostname
 *   \$  -> prompt char ($)
 *   \n  -> newline
 *   \e  -> ESC (start of ANSI color sequences)
 *   \\  -> backslash
 *   \w  -> full cwd (home abbreviated to ~ for non-root users)
 *   \W  -> basename of cwd (with home-abbrev rules applied)
 *
 * Minimal builtin line editor (termios-based on POSIX) with history recall.
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#include "exec.h"
#include "history.h"
#include "env.h"
#include "alias.h"

extern int alias_count;
extern char *names[];
extern char *values[];

/* Build-time defines from Meson (fall back to safe defaults) */
#ifndef KSH_RELEASE
#define KSH_RELEASE "unknown"
#endif
#ifndef KSH_TARGET
#define KSH_TARGET "unknown-target"
#endif
#ifndef KSH_CURRENT_YEAR
#define KSH_CURRENT_YEAR "1970"
#endif

/* SIGINT handling */
static volatile sig_atomic_t got_sigint = 0;
static void sigint_handler(int signo) {
    (void)signo;
    got_sigint = 1;
}

/* Helpers to obtain username and hostname in a portable manner. */
/* Returns pointer to a static buffer (do not free). */
static const char *get_username(void) {
    static char unamebuf[256];
    const char *u = getenv("USER");
    if (!u) u = getenv("USERNAME"); /* Windows compatibility */
    if (u && u[0] != '\0') {
        strncpy(unamebuf, u, sizeof(unamebuf) - 1);
        unamebuf[sizeof(unamebuf) - 1] = '\0';
        return unamebuf;
    }
    /* Try getpwuid if available */
    struct passwd *pw = getpwuid(geteuid());
    if (pw && pw->pw_name && pw->pw_name[0] != '\0') {
        strncpy(unamebuf, pw->pw_name, sizeof(unamebuf) - 1);
        unamebuf[sizeof(unamebuf) - 1] = '\0';
        return unamebuf;
    }
    /* Fallback string */
    strncpy(unamebuf, "I have no name!", sizeof(unamebuf) - 1);
    unamebuf[sizeof(unamebuf) - 1] = '\0';
    return unamebuf;
}

/* Returns pointer to a static buffer; if hostname unavailable returns empty string. */
static const char *get_hostname(void) {
    static char hostbuf[256];
    hostbuf[0] = '\0';
    if (gethostname(hostbuf, sizeof(hostbuf)) == 0) {
        hostbuf[sizeof(hostbuf) - 1] = '\0';
        if (hostbuf[0] != '\0') return hostbuf;
    }
    const char *h = getenv("HOSTNAME");
    if (h && h[0] != '\0') {
        strncpy(hostbuf, h, sizeof(hostbuf) - 1);
        hostbuf[sizeof(hostbuf) - 1] = '\0';
        return hostbuf;
    }
    return "";
}

/* Return user's home directory path (or empty string) */
static const char *get_home_dir(void) {
    static char homebuf[PATH_MAX];
    const char *h = getenv("HOME");
    if (h && h[0] != '\0') {
        strncpy(homebuf, h, sizeof(homebuf) - 1);
        homebuf[sizeof(homebuf) - 1] = '\0';
        return homebuf;
    }
    struct passwd *pw = getpwuid(geteuid());
    if (pw && pw->pw_dir && pw->pw_dir[0] != '\0') {
        strncpy(homebuf, pw->pw_dir, sizeof(homebuf) - 1);
        homebuf[sizeof(homebuf) - 1] = '\0';
        return homebuf;
    }
    homebuf[0] = '\0';
    return homebuf;
}

/* Get current working directory into out (size outlen). Returns 0 on success, -1 on error. */
static int get_cwd_safe(char *out, size_t outlen) {
    if (!getcwd(out, outlen)) {
        /* getcwd failed; try PWD env as fallback */
        const char *pwd = getenv("PWD");
        if (pwd && pwd[0] != '\0') {
            strncpy(out, pwd, outlen - 1);
            out[outlen - 1] = '\0';
            return 0;
        }
        out[0] = '\0';
        return -1;
    }
    return 0;
}

/* Abbreviate path: if path equals home and NOT running as root -> "~"
 * else if path is inside home and NOT root -> "~/<rest>"
 * else return full path or basename depending on mode.
 *
 * mode: 0 -> full path with home abbrev (used by \w)
 *       1 -> basename with home-abbrev rules (used by \W and default folder shown)
 */
static void format_path_abbrev(const char *path, char *out, size_t outlen, int mode) {
    const char *home = get_home_dir();
    bool have_home = (home && home[0] != '\0');
    uid_t euid = geteuid();
    bool is_root = (euid == 0);

    if (mode == 0) {
        /* full path, possibly abbreviated */
        if (have_home && !is_root) {
            size_t homelen = strlen(home);
            if (strncmp(path, home, homelen) == 0) {
                if (path[homelen] == '\0') {
                    /* equals home */
                    snprintf(out, outlen, "~");
                    return;
                } else if (path[homelen] == '/') {
                    /* inside home */
                    size_t restlen = strlen(path + homelen);
                    if (restlen + 2 <= outlen) { /* "~" + "/" + rest */
                        snprintf(out, outlen, "~%s", path + homelen);
                        return;
                    } else {
                        /* not enough space, fall back to truncated */
                    }
                }
            }
        }
        /* default: copy full path */
        strncpy(out, path, outlen - 1);
        out[outlen - 1] = '\0';
        return;
    } else {
        /* basename with home-abbrev rules */
        if (have_home && !is_root) {
            size_t homelen = strlen(home);
            if (strncmp(path, home, homelen) == 0) {
                if (path[homelen] == '\0') {
                    /* equals home */
                    snprintf(out, outlen, "~");
                    return;
                } else if (path[homelen] == '/') {
                    /* inside home: last component of rest */
                    const char *rest = path + homelen + 1; /* skip '/' */
                    const char *last = strrchr(rest, '/');
                    if (!last) {
                        /* rest is single component */
                        strncpy(out, rest, outlen - 1);
                        out[outlen - 1] = '\0';
                        return;
                    } else {
                        const char *b = last + 1;
                        strncpy(out, b, outlen - 1);
                        out[outlen - 1] = '\0';
                        return;
                    }
                }
            }
        }
        /* else: basename of full path */
        if (strcmp(path, "/") == 0) {
            strncpy(out, "/", outlen - 1);
            out[outlen - 1] = '\0';
            return;
        }
        const char *last = strrchr(path, '/');
        if (!last) {
            strncpy(out, path, outlen - 1);
            out[outlen - 1] = '\0';
            return;
        } else {
            const char *b = last + 1;
            if (*b == '\0') {
                /* path ends with '/', remove trailing slashes and retry */
                size_t plen = strlen(path);
                while (plen > 1 && path[plen - 1] == '/') plen--;
                /* find previous slash */
                size_t i = plen;
                while (i > 0 && path[i-1] != '/') i--;
                if (i == plen) {
                    strncpy(out, "/", outlen - 1);
                    out[outlen - 1] = '\0';
                    return;
                }
                const char *bb = path + i;
                strncpy(out, bb, outlen - 1);
                out[outlen - 1] = '\0';
                return;
            } else {
                strncpy(out, b, outlen - 1);
                out[outlen - 1] = '\0';
                return;
            }
        }
    }
}

/* Expand PS1-like escapes into out buffer, safe for given outlen.
 * Recognizes: \u \h \$ \n \e \\ \w \W
 * If hostname is empty, \h expands to nothing.
 */
static void expand_ps1(const char *ps1, char *out, size_t outlen, const char *username, const char *hostname) {
    size_t o = 0;
    char tmpbuf[PATH_MAX * 2];
    char cwd[PATH_MAX];
    cwd[0] = '\0';
    (void)get_cwd_safe(cwd, sizeof(cwd));
    for (size_t i = 0; ps1[i] != '\0' && o + 1 < outlen; ++i) {
        char c = ps1[i];
        if (c == '\\' && ps1[i+1] != '\0') {
            ++i;
            char esc = ps1[i];
            const char *rep = NULL;
            char tmp[2] = {0,0};
            switch (esc) {
                case 'u':
                    rep = username;
                    break;
                case 'h':
                    rep = (hostname && hostname[0] != '\0') ? hostname : "";
                    break;
                case '$':
                    tmp[0] = '$'; rep = tmp; break;
                case 'n':
                    tmp[0] = '\n'; rep = tmp; break;
                case 'e':
                    tmp[0] = '\x1b'; rep = tmp; break;
                case '\\':
                    tmp[0] = '\\'; rep = tmp; break;
                case 'w':
                    /* full cwd with home abbrev for non-root */
                    format_path_abbrev(cwd, tmpbuf, sizeof(tmpbuf), 0);
                    rep = tmpbuf;
                    break;
                case 'W':
                    /* basename with home-abbrev rules */
                    format_path_abbrev(cwd, tmpbuf, sizeof(tmpbuf), 1);
                    rep = tmpbuf;
                    break;
                default:
                    /* unknown escape -> keep backslash + char */
                    if (o + 2 < outlen) {
                        out[o++] = '\\';
                        out[o++] = esc;
                    }
                    rep = NULL;
                    break;
            }
            if (rep) {
                size_t rlen = strlen(rep);
                size_t copy = rlen;
                if (o + copy >= outlen) copy = outlen - 1 - o;
                if (copy > 0) {
                    memcpy(out + o, rep, copy);
                    o += copy;
                }
            }
        } else {
            out[o++] = c;
        }
    }
    out[o] = '\0';
}

/* Build the prompt string into dst (of dstlen). If PS1 env var present, expand it
 * with supported escapes. Otherwise build default: [username@hostname folder] $ with colors.
 */
static void build_prompt(char *dst, size_t dstlen, const char *username, const char *hostname) {
    const char *ps1 = getenv("PS1");
    bool use_tty_colors = isatty(STDOUT_FILENO);
    char folder[PATH_MAX];
    char cwd[PATH_MAX];
    cwd[0] = '\0';
    get_cwd_safe(cwd, sizeof(cwd));
    format_path_abbrev(cwd, folder, sizeof(folder), 1); /* basename with home-abbrev rules */

    if (ps1 && ps1[0] != '\0') {
        expand_ps1(ps1, dst, dstlen, username, hostname);
        return;
    }

    const char *clr_user = use_tty_colors ? "\x1b[32m" : ""; /* green */
    const char *clr_host = use_tty_colors ? "\x1b[36m" : ""; /* cyan */
    const char *clr_folder = use_tty_colors ? "\x1b[33m" : ""; /* yellow */
    const char *clr_reset = use_tty_colors ? "\x1b[0m"  : "";

    if (hostname && hostname[0] != '\0') {
        /* [user@host folder] $ */
        snprintf(dst, dstlen, "[%s%s%s@%s%s%s %s%s%s] $ ",
                 clr_user, username, clr_reset,
                 clr_host, hostname, clr_reset,
                 clr_folder, folder, clr_reset);
    } else {
        /* [user folder] $ */
        snprintf(dst, dstlen, "[%s%s%s %s%s%s] $ ",
                 clr_user, username, clr_reset,
                 clr_folder, folder, clr_reset);
    }
}

#ifndef _WIN32
/* POSIX: simple line reader with basic editing and history navigation.
 * Returns:
 *  1 - read a line successfully (buf filled, NUL-terminated)
 *  0 - EOF (caller should exit)
 * -1 - input cancelled (Ctrl-C) -> caller should continue loop and show prompt
 */
static int read_line_posix(char *buf, size_t buflen, const char *prompt) {
    #include <termios.h>
    struct termios orig, raw;
    if (tcgetattr(STDIN_FILENO, &orig) == -1) {
        if (!fgets(buf, buflen, stdin)) return 0;
        size_t len = strlen(buf);
        if (len && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[len-1] = '\0';
        return 1;
    }
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        if (!fgets(buf, buflen, stdin)) return 0;
        size_t len = strlen(buf);
        if (len && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[len-1] = '\0';
        return 1;
    }

    size_t len = 0;
    size_t prev_display_len = 0;
    size_t prompt_len = strlen(prompt);
    int history_index = history_count; /* one past last */
    bool done = false;

    if (write(STDOUT_FILENO, prompt, prompt_len) == -1) { }

    while (!done) {
        unsigned char c;
        ssize_t r = read(STDIN_FILENO, &c, 1);
        if (r == 0) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
            return 0;
        }
        if (r < 0) {
            if (errno == EINTR) {
                if (got_sigint) {
                    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
                    write(STDOUT_FILENO, "\n", 1);
                    got_sigint = 0;
                    return -1;
                }
                continue;
            } else {
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
                return 0;
            }
        }

        if (c == 0x03) { /* Ctrl-C */
            write(STDOUT_FILENO, "\n", 1);
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
            return -1;
        } else if (c == '\r' || c == '\n') {
            write(STDOUT_FILENO, "\n", 1);
            buf[len] = '\0';
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
            return 1;
        } else if (c == 0x7f || c == 0x08) {
            if (len > 0) {
                len--;
                const char *bs = "\b \b";
                write(STDOUT_FILENO, bs, 3);
            }
        } else if (c == 0x1b) {
            unsigned char seq[2] = {0,0};
            ssize_t r1 = read(STDIN_FILENO, &seq[0], 1);
            if (r1 <= 0) continue;
            ssize_t r2 = read(STDIN_FILENO, &seq[1], 1);
            if (r2 <= 0) continue;
            if (seq[0] == '[') {
                if (seq[1] == 'A') {
                    /* Up arrow */
                    if (history_count == 0) continue;
                    if (history_index > 0) history_index--;
                    const char *hline = history[history_index];
                    size_t hlen = strlen(hline);
                    if (hlen >= buflen) hlen = buflen - 1;
                    len = hlen;
                    memcpy(buf, hline, hlen);
                    buf[len] = '\0';
                    size_t now_display_len = prompt_len + len;
                    write(STDOUT_FILENO, "\r", 1);
                    write(STDOUT_FILENO, prompt, prompt_len);
                    write(STDOUT_FILENO, buf, len);
                    if (prev_display_len > now_display_len) {
                        size_t pad = prev_display_len - now_display_len;
                        for (size_t i = 0; i < pad; ++i) write(STDOUT_FILENO, " ", 1);
                    }
                    prev_display_len = now_display_len;
                } else if (seq[1] == 'B') {
                    /* Down arrow */
                    if (history_count == 0) continue;
                    if (history_index < history_count - 1) {
                        history_index++;
                        const char *hline = history[history_index];
                        size_t hlen = strlen(hline);
                        if (hlen >= buflen) hlen = buflen - 1;
                        len = hlen;
                        memcpy(buf, hline, hlen);
                        buf[len] = '\0';
                    } else {
                        history_index = history_count;
                        len = 0;
                        buf[0] = '\0';
                    }
                    size_t now_display_len = prompt_len + len;
                    write(STDOUT_FILENO, "\r", 1);
                    write(STDOUT_FILENO, prompt, prompt_len);
                    write(STDOUT_FILENO, buf, len);
                    if (prev_display_len > now_display_len) {
                        size_t pad = prev_display_len - now_display_len;
                        for (size_t i = 0; i < pad; ++i) write(STDOUT_FILENO, " ", 1);
                    }
                    prev_display_len = now_display_len;
                }
            }
        } else if (c >= 0x20 && c < 0x7f) {
            if (len + 1 < buflen) {
                buf[len++] = (char)c;
                char ch = (char)c;
                write(STDOUT_FILENO, &ch, 1);
                prev_display_len = prompt_len + len;
            }
        } else {
            /* ignore */
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    return 0;
}
#endif

/* Fallback read_line for systems without termios (Windows builds) */
static int read_line_fgets(char *buf, size_t buflen, const char *prompt) {
    fputs(prompt, stdout);
    fflush(stdout);
    if (!fgets(buf, buflen, stdin)) {
        if (feof(stdin)) {
            fputs("\n", stdout);
            return 0;
        }
        return -1;
    }
    size_t len = strlen(buf);
    if (len && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[len-1] = '\0';
    return 1;
}

/* Portable wrapper */
static int read_line(char *buf, size_t buflen, const char *prompt) {
#ifndef _WIN32
    return read_line_posix(buf, buflen, prompt);
#else
    return read_line_fgets(buf, buflen, prompt);
#endif
}

/* Forward-declare helper used by `source` builtin too */
int shell_eval_line(const char *line) {
    if (!line) return -1;
    char buf[512];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    /* Trim trailing newline/carriage return */
    size_t len = strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
        buf[--len] = '\0';
    }
    /* Ignore empty lines */
    if (len == 0) return 0;
    history_add(buf);
    char *argv[32];
    int argc = 0;
    char *tok = strtok(buf, " ");
    while (tok && argc < 31) {
        argv[argc++] = tok;
        tok = strtok(NULL, " ");
    }
    if (argc == 0) return 0;
    argv[argc] = NULL;
    /* Alias expansion */
    for (int i = 0; i < alias_count; ++i) {
        if (strcmp(argv[0], names[i]) == 0) {
            argv[0] = values[i];
            break;
        }
    }
    /* Builtins handled inline */
    if (strcmp(argv[0], "history") == 0) { history_show(); return 0; }
    if (strcmp(argv[0], "export") == 0 && argc == 2) { char *eq = strchr(argv[1], '='); if (eq) { *eq = 0; env_export(argv[1], eq + 1); } return 0; }
    if (strcmp(argv[0], "unset") == 0 && argc == 2) { env_unset(argv[1]); return 0; }
    if (strcmp(argv[0], "env") == 0) { env_show(); return 0; }
    if (strcmp(argv[0], "alias") == 0) { if (argc == 3) alias_set(argv[1], argv[2]); alias_show(); return 0; }
    if (strcmp(argv[0], "unalias") == 0 && argc == 2) { alias_unset(argv[1]); return 0; }

    if (exec_builtin(argv[0], argc, argv) == -1) {
        printf("Unknown command: %s\n", argv[0]);
    }
    return 0;
}

void shell_start(const char *version) {
    /* For fastfetch / compatibility */
    setenv("KSH_VERSION", version ? version : KSH_RELEASE, 1);

    /* Install SIGINT handler */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* Interactive loop using our portable read_line */
    char buf[512];
    char prompt[512];
    const char *username = get_username();
    const char *hostname = get_hostname();

    /* Line-buffer stdout for speed when not redirected */
    setvbuf(stdout, NULL, _IOLBF, 0);

    for (;;) {
        username = get_username();
        hostname = get_hostname();

        build_prompt(prompt, sizeof(prompt), username, hostname);

        int rl = read_line(buf, sizeof(buf), prompt);
        if (rl == 0) {
            /* EOF -> exit */
            break;
        } else if (rl == -1) {
            /* interrupted (Ctrl-C) -> show fresh prompt */
            continue;
        } else {
            shell_eval_line(buf);
        }
    }

    /* Cleanup or finalization can go here */
}
