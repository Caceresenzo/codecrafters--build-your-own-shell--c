#include <string.h>
#include <stdlib.h>

#include "shell.h"

static vector_t g_history;

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
