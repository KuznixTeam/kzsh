#ifndef ALIAS_H
#define ALIAS_H
#define ALIAS_MAX 100
extern int alias_count;
extern char *names[ALIAS_MAX];
extern char *values[ALIAS_MAX];
void alias_set(const char *name, const char *value);
void alias_unset(const char *name);
void alias_show();
#endif // ALIAS_H
