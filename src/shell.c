#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "exec.h"
#include "history.h"
#include "env.h"
#include "alias.h"
#include <stddef.h>
extern int alias_count;
extern char *names[];
extern char *values[];

// Forward-declare helper used by `source` builtin too
int shell_eval_line(const char *line) {
    // This minimal evaluator only supports tokenization, alias expansion,
    // and invoking builtins or external commands (same as the loop used to).
    if (!line) return -1;
    char buf[512];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    // Ignore empty lines
    if (strlen(buf) == 0) return 0;
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
    // Alias expansion
    for (int i = 0; i < alias_count; ++i) {
        extern char *names[];
        extern char *values[];
        if (strcmp(argv[0], names[i]) == 0) {
            argv[0] = values[i];
            break;
        }
    }
    // Handle a few builtins directly here (history/export/unset/env/alias handled elsewhere)
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
    // For fastfetch output compatibility
    setenv("KSH_VERSION", version, 1);
    // Load only .kshrc (do not auto-source .bashrc or .zshrc). Users who want
    // to load other rc files should add a line to their .kshrc such as
    // `source ~/.bashrc` or `source ~/.zshrc`.
    char *home = getenv("HOME");
    const char *rcfile = ".kshrc";
    if (home) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", home, rcfile);
        FILE *rc = fopen(path, "r");
        if (rc) {
            printf("Loading %s...\n", rcfile);
            char line[512];
            while (fgets(line, sizeof(line), rc)) {
                // strip newline
                line[strcspn(line, "\n")] = 0;
                shell_eval_line(line);
            }
            fclose(rc);
        }
    }

    // Simple shell main loop (demo)
    char line[256];
    char hostname[128] = "";
    char *user = getenv("USER");
    gethostname(hostname, sizeof(hostname));
    while (1) {
        char cwd[512];
        getcwd(cwd, sizeof(cwd));
        // Abbreviate the home directory to ~ or ~/subpath for nicer prompts
        static char pwd_display_buf[512];
        const char *pwd_display = cwd;
        if (home) {
            size_t home_len = strlen(home);
            if (strncmp(cwd, home, home_len) == 0) {
                if (cwd[home_len] == '\0') {
                    // exact home
                    pwd_display = "~";
                } else if (cwd[home_len] == '/') {
                    // subpath under home
                    snprintf(pwd_display_buf, sizeof(pwd_display_buf), "~%s", cwd + home_len);
                    pwd_display = pwd_display_buf;
                }
            }
        } else if (strncmp(cwd, "/root", 5) == 0) {
            // Fallback: if HOME isn't set but we're under /root, show ~ prefix
            if (cwd[5] == '\0') {
                pwd_display = "~";
            } else if (cwd[5] == '/') {
                snprintf(pwd_display_buf, sizeof(pwd_display_buf), "~%s", cwd + 5);
                pwd_display = pwd_display_buf;
            }
        }
        // Color codes: user (cyan), @ (default), hostname (green), : (default), path (yellow), $ (default)
        printf("\033[36m%s\033[0m@\033[32m%s\033[0m:\033[33m%s\033[0m$ ", user ? user : "user", hostname, pwd_display);
        if (!fgets(line, sizeof(line), stdin)) break;
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;
        shell_eval_line(line);
    }
}
