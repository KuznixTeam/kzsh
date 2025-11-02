#ifndef HISTORY_H
#define HISTORY_H

void history_add(const char *line);
void history_show(void);

/* Accessors for history (use these instead of exporting internal arrays) */
int history_count_get(void);
const char *history_get(int index);

#endif // HISTORY_H
