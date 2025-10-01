#include "../include/alias.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int alias_count = 0;
#define ALIAS_MAX 100
char *names[ALIAS_MAX];
char *values[ALIAS_MAX];
void alias_set(const char *name, const char *value) {
    for (int i = 0; i < alias_count; ++i) {
        if (strcmp(names[i], name) == 0) {
            free(values[i]);
            values[i] = strdup(value);
            return;
        }
    }
    if (alias_count < ALIAS_MAX) {
        names[alias_count] = strdup(name);
        values[alias_count++] = strdup(value);
    }
}
void alias_unset(const char *name) {
    for (int i = 0; i < alias_count; ++i) {
        if (strcmp(names[i], name) == 0) {
            free(names[i]);
            free(values[i]);
            for (int j = i; j < alias_count - 1; ++j) {
                names[j] = names[j+1];
                values[j] = values[j+1];
            }
            --alias_count;
            return;
        }
    }
}
void alias_show() {
    for (int i = 0; i < alias_count; ++i) {
        printf("alias %s='%s'\n", names[i], values[i]);
    }
}
