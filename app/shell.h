#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include <limits.h>
#include <stddef.h>

typedef struct {
    bool valid;
    int output;
    int error;
} io_t;

typedef void (*builtin_t)(int, char **, io_t);

builtin_t builtin_find(const char *name);

bool locate(const char *program, char output[static PATH_MAX]);

size_t strlen_or(const char *str, char alternative_end);

typedef struct
{
    int file_descriptor;
    char *path;
    bool append;
} redirect_t;

typedef struct
{
    char **argv;
    int argc;
    redirect_t *redirects;
    size_t redirect_count;
} parsed_line_t;

parsed_line_t line_parse(const char *line);
void line_free(parsed_line_t *parsed_line);

io_t io_open(redirect_t *redirects, int redirect_count);
void io_close(io_t *io);

#endif
