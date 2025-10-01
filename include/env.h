#ifndef ENV_H
#define ENV_H

void env_export(const char *name, const char *value);
void env_unset(const char *name);
void env_show();

#endif // ENV_H
