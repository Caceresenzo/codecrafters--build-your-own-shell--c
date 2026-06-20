#include "shell.h"

#include <string.h>
#include <stdlib.h>

char *strjoin(char **strings, size_t count, const char *separator)
{
    if (count == 0)
        return (strdup(""));

    size_t total_length = 0;
    size_t separator_length = strlen(separator);

    for (size_t index = 0; index < count; ++index)
    {
        total_length += strlen(strings[index]);

        if (index < count - 1)
            total_length += separator_length;
    }

    char *result = malloc(total_length + 1);
    if (!result)
        return (NULL);

    size_t offset = 0;
    for (size_t index = 0; index < count; ++index)
    {
        size_t length = strlen(strings[index]);
        memcpy(result + offset, strings[index], length);
        offset += length;

        if (index < count - 1)
        {
            memcpy(result + offset, separator, separator_length);
            offset += separator_length;
        }
    }

    result[total_length] = '\0';

    return (result);
}