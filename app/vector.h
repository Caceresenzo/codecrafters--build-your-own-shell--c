#ifndef STRING_H
#define STRING_H

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
void vector_destroy(vector_t *vector);
void vector_resize(vector_t *vector, size_t new_capacity);
void vector_shrink(vector_t *vector);
void *vector_get(vector_t *vector, size_t index);
void vector_append(vector_t *vector, const void *item);
bool vector_is_empty(const vector_t *vector);
void vector_clear(vector_t *vector);
bool vector_pop(vector_t *vector);
void vector_add_all_iterate(vector_t *vector, const void *item, size_t count);
bool vector_contains(const vector_t *vector, const void *item, int (*comparator)(const void *, const void *));
void vector_sort(const vector_t *vector, int (*comparator)(const void *, const void *));

int string_compare(const void *left, const void *right);
int string_compare_short_first(const void *left, const void *right);

#endif
