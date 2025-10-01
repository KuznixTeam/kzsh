#ifndef UTILS_H
#define UTILS_H
#ifdef __cplusplus
#include <string>
void print_banner(const std::string& version);
bool load_kshrc();
#else
#include <string.h>
void print_banner(const char *version);
#endif
#endif // UTILS_H
