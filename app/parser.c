#include "shell.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>

#include "iterator.h"
#include "vector.h"

#define END '\0'
#define SPACE ' '
#define SINGLE '\''
#define DOUBLE '"'
#define BACKSLASH '\\'
#define GREATER_THAN '>'
#define PIPE '|'

typedef struct
{
    string_iterator_t iterator;
    vector_t commands;
    vector_t arguments;
    vector_t redirects;
} parser_t;

char *parse_next_argument(parser_t *);

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

static void parse_redirect(
    parser_t *parser,
    int file_descriptor)
{
    if (file_descriptor != STDOUT_FILENO && file_descriptor != STDERR_FILENO)
        file_descriptor = -1;

    char character = string_iterator_next(&parser->iterator);
    if (!character)
        return;

    bool append = character == GREATER_THAN;
    if (append)
        string_iterator_next(&parser->iterator);

    char *path = parse_next_argument(parser);

    redirect_t redirect = {
        .file_descriptor = file_descriptor,
        .path = path,
        .append = append,
    };

    vector_append(&parser->redirects, &redirect);
}

static void parse_pipe(
    parser_t *parser)
{
    char *null = NULL;
    vector_append(&parser->arguments, &null);

    vector_shrink(&parser->arguments);
    vector_shrink(&parser->redirects);

    parsed_line_t parsed_line = {
        .argv = parser->arguments.pointer,
        .argc = parser->arguments.length - 1,
        .redirects = parser->redirects.pointer,
        .redirect_count = parser->redirects.length,
    };

    vector_append(&parser->commands, &parsed_line);

    parser->arguments = vector_initialize(sizeof(char *));
    parser->redirects = vector_initialize(sizeof(redirect_t));
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

        case GREATER_THAN:
        {
            parse_redirect(parser, STDOUT_FILENO);

            break;
        }

        case PIPE:
        {
            parse_pipe(parser);

            break;
        }

        default:
        {
            if (isdigit(character) && string_iterator_peek(iterator) == GREATER_THAN)
            {
                string_iterator_next(iterator);
                parse_redirect(parser, character - '0');
            }
            else
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

vector_t line_parse(const char *line)
{
    parser_t parser = {
        .iterator = string_iterator_from(line),
        .commands = vector_initialize(sizeof(parsed_line_t)),
        .arguments = vector_initialize(sizeof(char *)),
        .redirects = vector_initialize(sizeof(redirect_t)),
    };

    string_iterator_first(&parser.iterator);

    char *argument;
    while (argument = parse_next_argument(&parser))
        vector_append(&parser.arguments, &argument);

    if (!vector_is_empty(&parser.arguments))
        parse_pipe(&parser);

    vector_destroy(&parser.arguments);
    vector_destroy(&parser.redirects);

    return parser.commands;
}

void line_free(parsed_line_t *parsed_line)
{
    {
        char **argv = parsed_line->argv;

        for (size_t index = 0; argv[index]; ++index)
            free(argv[index]);

        free(argv);

        parsed_line->argv = NULL;
        parsed_line->argc = -1;
    }

    {
        redirect_t *redirects = parsed_line->redirects;
        int redirect_count = parsed_line->redirect_count;

        for (size_t index = 0; index < redirect_count; ++index)
            free(redirects[index].path);

        free(redirects);

        parsed_line->redirects = NULL;
        parsed_line->redirect_count = 0;
    }
}
