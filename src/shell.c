/*
 * Portable shell front-end (no readline)
 *
 * This version uses a simple fgets loop for input so the codebase
 * builds and runs on Windows, macOS, FreeBSD, NetBSD, GhostBSD, Linux,
 * and other systems without requiring the readline library.
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "exec.h"
#include "history.h"
#include "env.h"
#include "alias.h"
#include <stddef.h>

extern int alias_count;
extern char *names[];
extern char *values[];

/* SIGINT handling: flag and handler at file scope so we don't create nested functions. */
static volatile sig_atomic_t got_sigint = 0;
static void sigint_handler(int signo) {
    (void)signo;
    got_sigint = 1;
}

/* Forward-declare helper used by `source` builtin too */
int shell_eval_line(const char *line) {
    if (!line) return -1;
    char buf[512];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    /* Trim leading/trailing whitespace */
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
    setenv("KSH_VERSION", version, 1);

    /* Install SIGINT handler */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* Interactive loop using fgets (portable) */
    char buf[512];
    for (;;) {
        /* Print prompt */
        printf("ksh> ");
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
                /* move to next prompt line */
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
