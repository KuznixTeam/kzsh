#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int builtin_echo(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        printf("%s%s", argv[i], (i < argc-1) ? " " : "\n");
    }
    return 0;
}

int builtin_true(int argc, char **argv) {
    return 0;
}

int builtin_false(int argc, char **argv) {
    return 1;
}

int builtin_cd(int argc, char **argv) {
    const char *dir = NULL;
    if (argc < 2) {
        dir = getenv("HOME");
        if (!dir) return -1;
    } else {
        dir = argv[1];
    }
    if (chdir(dir) != 0) {
        fprintf(stderr, "cd: %s: %s\n", dir, strerror(errno));
        return -1;
    }
    return 0;
}

int builtin_source(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "source: filename required\n");
        return -1;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "source: cannot open %s: %s\n", argv[1], strerror(errno));
        return -1;
    }
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        // strip newline
        line[strcspn(line, "\n")] = 0;
        shell_eval_line(line);
    }
    fclose(f);
    return 0;
}

// Add more builtins as needed
