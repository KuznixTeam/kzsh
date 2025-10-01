#include "../include/history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HISTORY_MAX 100
static char *history[HISTORY_MAX];
static int history_count = 0;

void history_add(const char *line) {
    if (history_count < HISTORY_MAX) {
        history[history_count++] = strdup(line);
    }
}

void history_show() {
    for (int i = 0; i < history_count; ++i) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}
