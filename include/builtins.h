#ifndef BUILTINS_H
#define BUILTINS_H

int builtin_echo(int argc, char **argv);
int builtin_true(int argc, char **argv);
int builtin_false(int argc, char **argv);
int builtin_cd(int argc, char **argv);
int builtin_source(int argc, char **argv);
// Add more builtins as needed

#endif // BUILTINS_H
