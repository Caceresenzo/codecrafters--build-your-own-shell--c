#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include <limits.h>

typedef void (*builtin_t)(int, char **);

builtin_t builtin_find(const char *name);

bool locate(const char *program, char output[static PATH_MAX]);

size_t strlen_or(const char *str, char alternative_end);

#endif
