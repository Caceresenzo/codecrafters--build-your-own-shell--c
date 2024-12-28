#include "vector.h"

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>

static char *to_byte_pointer_with_offset(vector_t *vector, size_t index)
{
    return (((char*) vector->pointer) + (index * vector->item_size));
}

vector_t vector_initialize(size_t item_size)
{
    return ((vector_t){
        .pointer = NULL,
        .item_size = item_size,
        .length = 0,
        .capacity = 0,
    });
}

void vector_destroy(vector_t *vector)
{
    free(vector->pointer);

    vector->pointer = 0;
    vector->length = 0;
    vector->capacity = 0;
}

void vector_resize(vector_t *vector, size_t new_capacity)
{
    char *pointer = realloc(vector->pointer, (new_capacity * vector->item_size));

    vector->pointer = pointer;
    vector->length = MIN(vector->length, new_capacity);
    vector->capacity = new_capacity;
}

void vector_shrink(vector_t *vector)
{
    vector_resize(vector, vector->length);
}

void vector_append(vector_t *vector, void *item)
{
    if (vector->length == vector->capacity)
        vector_resize(vector, vector->capacity + 20); // TODO Magic number

    memmove(to_byte_pointer_with_offset(vector, vector->length), item, vector->item_size);
    ++vector->length;
}

bool vector_is_empty(const vector_t *vector)
{
    return (vector->length == 0);
}
