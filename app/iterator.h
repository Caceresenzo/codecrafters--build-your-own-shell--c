#ifndef ITERATOR_H
# define ITERATOR_H

#include <stddef.h>

typedef struct
{
    const char *pointer;
    size_t length;
    size_t index;
} string_iterator_t;

string_iterator_t string_iterator_from(const char *string);
char string_iterator_remaining(const string_iterator_t *iterator);
char string_iterator_first(string_iterator_t *iterator);
char string_iterator_current(string_iterator_t *iterator);
char string_iterator_next(string_iterator_t *iterator);
char string_iterator_peek(const string_iterator_t *iterator);

#endif
