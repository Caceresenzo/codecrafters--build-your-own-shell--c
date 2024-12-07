#include "shell.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#define END '\0'
#define SPACE ' '
#define SINGLE '\''

static void argv_grow(
    char ***argv,
    size_t *argc,
    char *builder,
    size_t *builder_length)
{
    if (!*builder)
        return;

    char *arg = malloc(*builder_length + 1);
    memcpy(arg, builder, *builder_length + 1);

    bzero(builder, *builder_length);
    *builder_length = 0;

    (*argv)[*argc] = arg;

    *argc += 1;
    *argv = realloc(*argv, (*argc + 1) * sizeof(char *));
    (*argv)[*argc] = NULL;
}

char **argv_parse(const char *line)
{
    size_t line_length = strlen(line);

    char *builder = calloc(line_length + 1, sizeof(char)); // TODO Grow automatically instead.
    size_t builder_length = 0;

    char **argv = calloc(1, sizeof(char *));
    size_t argc = 0;

    for (size_t index = 0; index < line_length; ++index)
    {
        char character = line[index];

        switch (character)
        {
        case SPACE:
        {
            argv_grow(&argv, &argc, builder, &builder_length);
            break;
        }

        case SINGLE:
        {
            for (++index; index < line_length; ++index)
            {
                character = line[index];

                if (character == SINGLE)
                    break;

                builder[builder_length++] = character;
            }

            break;
        }

        default:
        {
            builder[builder_length++] = character;

            break;
        }
        }
    }

    argv_grow(&argv, &argc, builder, &builder_length);

    free(builder);
    return (argv);
}

void argv_free(char **argv)
{
    for (size_t index = 0; argv[index]; ++index)
        free(argv[index]);

    free(argv);
}
