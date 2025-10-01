#include "../include/env.h"
#include <stdio.h>
#include <stdlib.h>

void env_export(const char *name, const char *value) {
    setenv(name, value, 1);
}

void env_unset(const char *name) {
    unsetenv(name);
}

void env_show() {
    extern char **environ;
    for (char **env = environ; *env; ++env) {
        printf("%s\n", *env);
    }
}
