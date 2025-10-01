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

void shell_start(const char *version) {
    printf("ksh-%s\n", version);

    // For fastfetch output compatibility
    setenv("KSH_VERSION", version, 1);
    // Load .kshrc, .bashrc, .zshrc if present
    char *home = getenv("HOME");
    const char *rcfiles[] = { ".kshrc", ".bashrc", ".zshrc" };
    if (home) {
        for (int i = 0; i < 3; ++i) {
            char rcfile[512];
            snprintf(rcfile, sizeof(rcfile), "%s/%s", home, rcfiles[i]);
            FILE *rc = fopen(rcfile, "r");
            if (rc) {
                printf("Loading %s...\n", rcfiles[i]);
                // TODO: parse and execute rc file
                fclose(rc);
            }
        }
    }
    // Simple shell main loop (demo)
    char line[256];
    char hostname[128] = "";
    char *user = getenv("USER");
    // 'home' already declared above
    gethostname(hostname, sizeof(hostname));
    while (1) {
        char cwd[512];
        getcwd(cwd, sizeof(cwd));
        // Show ~ for home dir, / for root
        const char *pwd_display = cwd;
        if (home && strcmp(cwd, home) == 0) {
            pwd_display = "~";
        } else if (strcmp(cwd, "/root") == 0) {
            pwd_display = "~";
        }
    // Color codes: user (cyan), @ (default), hostname (green), : (default), path (yellow), $ (default)
    printf("\033[36m%s\033[0m@\033[32m%s\033[0m:\033[33m%s\033[0m$ ", user ? user : "user", hostname, pwd_display);
        if (!fgets(line, sizeof(line), stdin)) break;
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;
        history_add(line);
        // Tokenize
        char *argv[32];
        int argc = 0;
        char *tok = strtok(line, " ");
        while (tok && argc < 31) {
            argv[argc++] = tok;
            tok = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        // Alias expansion
        if (argc > 0) {
            for (int i = 0; i < alias_count; ++i) {
                extern char *names[];
                extern char *values[];
                if (strcmp(argv[0], names[i]) == 0) {
                    argv[0] = values[i];
                    break;
                }
            }
        }
        // Builtin: history
        if (strcmp(argv[0], "history") == 0) {
            history_show();
            continue;
        }
        // Builtin: export
        if (strcmp(argv[0], "export") == 0 && argc == 2) {
            char *eq = strchr(argv[1], '=');
            if (eq) {
                *eq = 0;
                env_export(argv[1], eq + 1);
            }
            continue;
        }
        // Builtin: unset
        if (strcmp(argv[0], "unset") == 0 && argc == 2) {
            env_unset(argv[1]);
            continue;
        }
        // Builtin: env
        if (strcmp(argv[0], "env") == 0) {
            env_show();
            continue;
        }
        // Builtin: alias
        if (strcmp(argv[0], "alias") == 0) {
            if (argc == 3) alias_set(argv[1], argv[2]);
            alias_show();
            continue;
        }
        // Builtin: unalias
        if (strcmp(argv[0], "unalias") == 0 && argc == 2) {
            alias_unset(argv[1]);
            continue;
        }
        if (exec_builtin(argv[0], argc, argv) == -1) {
            printf("Unknown command: %s\n", argv[0]);
        }
    }
}
