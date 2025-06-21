#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "shell.h"

static vector_t g_history;
static size_t g_last_history_append_index = 0;

void history_initialize()
{
    g_history = vector_initialize(sizeof(char *));
}

void history_destroy()
{
    for (size_t index = 0; index < g_history.length; ++index)
    {
        char **line = vector_get(&g_history, index);
        if (line)
            free(*line);
    }

    vector_destroy(&g_history);
}

void history_add(const char *line)
{
    line = strdup(line);

    if (line)
        vector_append(&g_history, &line);
}

size_t history_size()
{
    return (g_history.length);
}

const char *history_get(size_t index)
{
    const char **line = vector_get(&g_history, index);

    return (line ? *line : NULL);
}

bool history_read(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
        return (false);

    char *line = NULL;
    size_t buffer_length = 0;
    while (getline(&line, &buffer_length, file) != -1)
    {
        size_t line_length = strlen(line);

        if (line_length && line[line_length - 1] == '\n')
        {
            line[line_length - 1] = '\0';
            --line_length;
        }

        if (line_length && line[line_length - 1] == '\r')
        {
            line[line_length - 1] = '\0';
            --line_length;
        }

        if (line_length)
            history_add(line);

        free(line);
        line = NULL;
    }

    fclose(file);

    return (true);
}

bool history_write(const char *path)
{
    FILE *file = fopen(path, "w");
    if (!file)
        return (false);

    size_t size = history_size();
    for (size_t index = 0; index < size; ++index)
    {
        const char *line = history_get(index);
        fprintf(file, "%s\n", line);
    }

    fclose(file);

    return (true);
}

bool history_append(const char *path)
{
    FILE *file = fopen(path, "w");
    if (!file)
        return (false);

    size_t size = history_size();
    for (size_t index = g_last_history_append_index; index < size; ++index)
    {
        const char *line = history_get(index);
        fprintf(file, "%s\n", line);
    }

    g_last_history_append_index = size;

    fclose(file);

    return (true);
}
