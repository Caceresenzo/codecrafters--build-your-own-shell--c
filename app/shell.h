#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include <limits.h>
#include <stddef.h>

#include "vector.h"

typedef struct
{
    bool valid;
    int output;
    int error;
} io_t;

typedef void (*builtin_t)(int, char **, io_t);

typedef struct
{
    const char *name;
    builtin_t function;
} builtin_entry_t;

extern builtin_entry_t g_builtins[];

builtin_t builtin_find(const char *name);

size_t strlen_or(const char *str, char alternative_end);
bool locate(const char *program, char output[static PATH_MAX]);
void write_string(const char *string);

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

typedef enum
{
    AR_NONE,
    AR_FOUND,
    AR_MORE,
} e_autocomplete_result;

void bell();

void shell_prompt();
e_autocomplete_result autocomplete(vector_t *line, bool bell_rung);

#endif
