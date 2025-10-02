#ifndef SHELL_H
#define SHELL_H

// Start the interactive shell
void shell_start(const char *version);

// Evaluate a single command line (used by the interactive loop and `source`)
int shell_eval_line(const char *line);

#endif // SHELL_H
