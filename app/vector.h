#ifndef STRING_H
# define STRING_H

#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    void *pointer;
    size_t item_size;
    size_t length;
    size_t capacity;
} vector_t;

vector_t vector_initialize(size_t item_size);
void vector_destroy(vector_t *builder);
void vector_resize(vector_t *builder, size_t new_capacity);
void vector_shrink(vector_t *builder);
void *vector_get(vector_t *vector, size_t index);
void vector_append(vector_t *builder, void *item);
bool vector_is_empty(const vector_t *builder);

#endif
