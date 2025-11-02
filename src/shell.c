/*
 * Portable shell front-end (no readline)
 *
 * Prompt and banner improvements:
 *  - Default prompt: {username}@{hostname}$ (falls back to "I have no name!" and/or username$ when missing)
 *  - PS1 environment variable supported. Recognized escapes:
 *      \u  -> username
 *      \h  -> hostname (omitted if hostname is empty)
 *      \$  -> prompt char ($)
 *      \n  -> newline
 *      \e  -> ESC (start of ANSI color sequences)
 *      \\  -> backslash
 *  - Colors enabled by default when stdout is a TTY (username green, host cyan).
 *  - Banner is printed at startup using compile-time defines.
 *
 * This file remains fgets-based and portable across platforms that provide
 * basic POSIX-ish APIs (getenv/getpwuid/gethostname). Where platform headers
 * are missing (Windows/MSYS), getenv("USERNAME") is used first for username.
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

#include "exec.h"
#include "history.h"
#include "env.h"
#include "alias.h"

extern int alias_count;
extern char *names[];
extern char *values[];

/* These macros are populated at build time by Meson. */
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
    /* Fallback string required by user */
    strncpy(unamebuf, "I have no name!", sizeof(unamebuf) - 1);
    unamebuf[sizeof(unamebuf) - 1] = '\0';
    return unamebuf;
}

/* Returns pointer to a static buffer; if hostname unavailable returns empty string. */
static const char *get_hostname(void) {
    static char hostbuf[256];
    hostbuf[0] = '\0';
    if (gethostname(hostbuf, sizeof(hostbuf)) == 0) {
        /* Ensure null-termination */
        hostbuf[sizeof(hostbuf) - 1] = '\0';
        if (hostbuf[0] != '\0') return hostbuf;
    }
    /* If gethostname fails, try environment as fallback */
    const char *h = getenv("HOSTNAME");
    if (h && h[0] != '\0') {
        strncpy(hostbuf, h, sizeof(hostbuf) - 1);
        hostbuf[sizeof(hostbuf) - 1] = '\0';
        return hostbuf;
    }
    /* Return empty string to indicate missing hostname */
    return "";
}

/* Expand PS1-like escapes into out buffer, safe for given outlen.
 * Recognizes: \u \h \$ \n \e \\  (and passes through other chars).
 * If hostname is empty, \h expands to nothing.
 */
static void expand_ps1(const char *ps1, char *out, size_t outlen, const char *username, const char *hostname) {
    size_t o = 0;
    for (size_t i = 0; ps1[i] != '\0' && o + 1 < outlen; ++i) {
        char c = ps1[i];
        if (c == '\\' && ps1[i+1] != '\0') {
            ++i;
            char esc = ps1[i];
            const char *rep = NULL;
            char tmp[2] = {0,0};
            switch (esc) {
                case 'u': rep = username; break;
                case 'h': rep = (hostname && hostname[0] != '\0') ? hostname : ""; break;
                case '$': tmp[0] = '$'; rep = tmp; break;
                case 'n': tmp[0] = '\n'; rep = tmp; break;
                case 'e': tmp[0] = '\x1b'; rep = tmp; break; /* ANSI ESC */
                case '\\': tmp[0] = '\\'; rep = tmp; break;
                default:
                    /* Unknown escape -> output as-is: backslash + char */
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
 * with supported escapes. Otherwise build default: username@hostname$ with color
 * when connected to a TTY.
 */
static void build_prompt(char *dst, size_t dstlen, const char *username, const char *hostname) {
    const char *ps1 = getenv("PS1");
    bool use_tty_colors = isatty(STDOUT_FILENO);
    if (ps1 && ps1[0] != '\0') {
        /* Expand PS1; user may include ANSI via \e[...]m sequences */
        expand_ps1(ps1, dst, dstlen, username, hostname);
        return;
    }

    /* Default formatted prompt with colors (if TTY) */
    const char *clr_user = use_tty_colors ? "\x1b[32m" : ""; /* green */
    const char *clr_host = use_tty_colors ? "\x1b[36m" : ""; /* cyan */
    const char *clr_reset = use_tty_colors ? "\x1b[0m"  : "";

    if (hostname && hostname[0] != '\0') {
        /* username@hostname$ */
        snprintf(dst, dstlen, "%s%s%s@%s%s%s$ ", 
                 clr_user, username, clr_reset,
                 clr_host, hostname, clr_reset);
    } else {
        /* hostname missing -> username$ */
        snprintf(dst, dstlen, "%s%s%s$ ", clr_user, username, clr_reset);
    }
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

    /* Print improved banner */
    printf("kzsh, version %s (%s)\n", KSH_RELEASE, KSH_TARGET);
    printf("Copyright (C) %s Kuznix\n", KSH_CURRENT_YEAR);
    printf("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\n");
    printf("This is free software; you can redistribute it and/or modify it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n\n");

    /* Install SIGINT handler */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* Interactive loop using fgets (portable) */
    char buf[512];
    char prompt[512];
    const char *username = get_username();
    const char *hostname = get_hostname();

    /* Line-buffer stdout for speed when not redirected */
    setvbuf(stdout, NULL, _IOLBF, 0);

    for (;;) {
        /* Recompute username/hostname each loop in case env/user context changed */
        username = get_username();
        hostname = get_hostname();

        /* Build prompt (PS1 override supported) */
        build_prompt(prompt, sizeof(prompt), username, hostname);

        /* Print prompt */
        fputs(prompt, stdout);
        fflush(stdout);

        /* Read a line */
        if (!fgets(buf, sizeof(buf), stdin)) {
            /* If fgets returned NULL, either EOF or error/interrupt */
            if (feof(stdin)) {
                /* End-of-file: exit cleanly */
                printf("\n");
                break;
            }
            /* If interrupted by SIGINT, clear the flag and continue */
            if (got_sigint) {
                got_sigint = 0;
                printf("\n");
                continue;
            }
            /* Other errors: break */
            break;
        }

        /* If SIGINT was received while reading, drop the current input */
        if (got_sigint) {
            got_sigint = 0;
            /* Discard the buffer contents and continue */
            continue;
        }

        /* Trim newline at end if present */
        size_t len = strlen(buf);
        if (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            buf[len - 1] = '\0';
        }

        /* Evaluate the line */
        shell_eval_line(buf);
    }

    /* Cleanup or finalization can go here */
}
