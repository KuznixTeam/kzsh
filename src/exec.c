#include "builtins.h"
#include <stdio.h>
#include <string.h>


#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


int exec_builtin(const char *cmd, int argc, char **argv) {
    if (strcmp(cmd, "echo") == 0) return builtin_echo(argc, argv);
    if (strcmp(cmd, "true") == 0) return builtin_true(argc, argv);
    if (strcmp(cmd, "false") == 0) return builtin_false(argc, argv);
    if (strcmp(cmd, "cd") == 0) return builtin_cd(argc, argv);
    if (strcmp(cmd, "source") == 0) return builtin_source(argc, argv);
    if (strcmp(cmd, "exit") == 0) {
        int code = (argc > 1) ? atoi(argv[1]) : 0;
        exit(code);
    }
    // Add more builtins here
    // If not a builtin, try to exec external binary
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmd, argv);
        perror("execvp");
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    } else {
        perror("fork");
        return -1;
    }
}
