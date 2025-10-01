#include "shell.h"
#include <stdio.h>
#include <string.h>

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

// Add more builtins as needed
