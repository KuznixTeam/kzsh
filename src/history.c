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
    } else {
        /* simple rolling: free oldest, shift left, append at end */
        free(history[0]);
        for (int i = 1; i < HISTORY_MAX; ++i) history[i-1] = history[i];
        history[HISTORY_MAX-1] = strdup(line);
    }
}

void history_show(void) {
    for (int i = 0; i < history_count; ++i) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

/* Accessor implementations */
int history_count_get(void) {
    return history_count;
}

const char *history_get(int index) {
    if (index < 0 || index >= history_count) return NULL;
    return history[index];
}
