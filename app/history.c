#include <string.h>

#include "shell.h"

static vector_t g_history;

void history_initialize()
{
    g_history = vector_initialize(sizeof(char *));
}

void history_add(const char *line)
{
    line = strdup(line);

    if (line)
        vector_append(&g_history, &line);
}
