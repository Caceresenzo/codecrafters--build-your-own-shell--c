#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "shell.h"

static void commit(vector_t *line, char *candidate)
{
    size_t length = strlen(candidate);

    write(STDOUT_FILENO, candidate, length);
    vector_add_all_iterate(line, candidate, length);

    char space = ' ';
    write(STDOUT_FILENO, &space, 1);
    vector_append(line, &space);
}

e_autocomplete_result autocomplete(vector_t *line)
{
    vector_t candidates = vector_initialize(sizeof(char *));

    for (builtin_entry_t *entry = g_builtins; entry->name; ++entry)
    {
        if (strncmp(line->pointer, entry->name, line->length) == 0)
        {
            char *candidate = strdup(entry->name + line->length);

            vector_append(&candidates, &candidate);
        }
    }

    e_autocomplete_result result;

    size_t length = candidates.length;
    if (length == 0)
    {
        result = AR_NONE;
    }
    else if (length == 1)
    {
        result = AR_FOUND;

        char **candidate = vector_get(&candidates, 0);
        commit(line, *candidate);
    }
    else
    {
        printf("TODO multiple candidates\n");
        result = AR_NONE;
    }

    for (size_t index = 0; index < candidates.length; ++index)
        free(*((char **)vector_get(&candidates, index)));

    vector_destroy(&candidates);

    return (result);
}
