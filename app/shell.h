#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include <limits.h>
#include <stddef.h>
#include <unistd.h>

#include "vector.h"

typedef struct
{
    bool valid;
    int output;
    int error;
} io_t;

typedef struct command_result_s
{
    int exit_code;
    bool exit_shell;
} command_result_t;

typedef command_result_t (*builtin_t)(int, char **, io_t);

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
void write_string_n(const char *string, size_t length);

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

vector_t line_parse(const char *line);
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

pid_t pipeline(vector_t commands);

void history_initialize();
void history_destroy();
void history_add(const char *line);
size_t history_size();
const char *history_get(size_t index);
bool history_read(const char *path);
bool history_write(const char *path);
bool history_append(const char *path);

#endif
