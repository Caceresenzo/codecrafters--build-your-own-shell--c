#include "iterator.h"

#include <string.h>

string_iterator_t string_iterator_from(const char *string)
{
    return ((string_iterator_t){
        .pointer = string,
        .length = strlen(string),
        .index = 0});
}

char string_iterator_remaining(const string_iterator_t *iterator)
{
    return (iterator->length - iterator->index);
}

char string_iterator_first(string_iterator_t *iterator)
{
    iterator->index = 0;

    return (*iterator->pointer);
}

char string_iterator_current(string_iterator_t *iterator)
{
    return (iterator->pointer[iterator->index]);
}

char string_iterator_next(string_iterator_t *iterator)
{
    if (iterator->index == iterator->length)
        return ('\0');

    return (iterator->pointer[++iterator->index]);
}

char string_iterator_peek(const string_iterator_t *iterator)
{
    if (iterator->index == iterator->length)
        return ('\0');

    return (iterator->pointer[iterator->index + 1]);
}
