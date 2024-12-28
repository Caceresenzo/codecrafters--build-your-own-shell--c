#include "shell.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include "iterator.h"
#include "vector.h"

#define END '\0'
#define SPACE ' '
#define SINGLE '\''
#define DOUBLE '"'
#define BACKSLASH '\\'

typedef struct
{
    string_iterator_t iterator;
    vector_t arguments;
} parser_t;

static char *build_string(vector_t *builder)
{
    char zero = '\0';
    vector_append(builder, &zero);

    char *string = strdup(builder->pointer);
    vector_destroy(builder);

    return (string);
}

static char map_backslash_character(char character)
{
    if (character == DOUBLE || character == BACKSLASH)
        return (character);

    return (END);
}

static void parse_backslash(
    string_iterator_t *iterator,
    vector_t *builder,
    bool in_quote)
{
    char character = string_iterator_next(iterator);
    if (!character)
        return;

    if (in_quote)
    {
        char mapped = map_backslash_character(character);

        if (mapped != END)
            character = mapped;
        else
        {
            char backslash = BACKSLASH;
            vector_append(builder, &backslash);
        }
    }

    vector_append(builder, &character);
}

char *parse_next_argument(
    parser_t *parser)
{
    string_iterator_t *iterator = &parser->iterator;

    vector_t builder = vector_initialize(sizeof(char));
    vector_resize(&builder, string_iterator_remaining(iterator) + 1);

    for (
        char character = string_iterator_current(iterator);
        character;
        character = string_iterator_next(iterator))
    {
        switch (character)
        {
        case SPACE:
        {
            if (!vector_is_empty(&builder))
                return (build_string(&builder));

            break;
        }

        case SINGLE:
        {
            while ((character = string_iterator_next(iterator)) && character != SINGLE)
                vector_append(&builder, &character);

            break;
        }

        case DOUBLE:
        {
            while ((character = string_iterator_next(iterator)) && character != DOUBLE)
            {
                if (character == BACKSLASH)
                    parse_backslash(iterator, &builder, true);
                else
                    vector_append(&builder, &character);
            }

            break;
        }

        case BACKSLASH:
        {
            parse_backslash(iterator, &builder, false);

            break;
        }

        default:
        {
            vector_append(&builder, &character);

            break;
        }
        }
    }

    if (!vector_is_empty(&builder))
        return (build_string(&builder));

    vector_destroy(&builder);
    return (NULL);
}

char **line_parse(const char *line)
{
    parser_t parser = {
        .iterator = string_iterator_from(line),
        .arguments = vector_initialize(sizeof(char *)),
    };

    string_iterator_first(&parser.iterator);

    char *argument;
    while (argument = parse_next_argument(&parser))
        vector_append(&parser.arguments, &argument);

    vector_append(&parser.arguments, &argument);

    vector_shrink(&parser.arguments);

    return (parser.arguments.pointer);
}

void line_free(char **argv)
{
    for (size_t index = 0; argv[index]; ++index)
        free(argv[index]);

    free(argv);
}
