#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "shell.h"

void bell()
{
    write(STDOUT_FILENO, "\a", 1);
}

static void commit(vector_t *line, char *candidate, bool has_more)
{
    size_t length = strlen(candidate);

    write(STDOUT_FILENO, candidate, length);
    vector_add_all_iterate(line, candidate, length);

    if (has_more || candidate[length - 1] == '/')
        return;

    char space = ' ';
    write(STDOUT_FILENO, &space, 1);
    vector_append(line, &space);
}

static void collect_builtins(vector_t *candidates, const char *prefix)
{
    size_t prefix_length = strlen(prefix);

    for (builtin_entry_t *entry = g_builtins; entry->name; ++entry)
    {
        if (strncmp(prefix, entry->name, prefix_length) == 0)
        {
            char *candidate = strdup(entry->name + prefix_length);
            vector_append(candidates, &candidate);
        }
    }
}

static void collect_executables(vector_t *candidates, const char *prefix)
{
    struct stat path_stat;

    const char *paths = getenv("PATH");
    if (!paths)
        return;

    size_t prefix_length = strlen(prefix);

    const char *directory_iterator = paths;
    while (true)
    {
        const char *directory = directory_iterator;

        size_t directory_length = strlen_or(directory, ':');
        directory_iterator += directory_length + 1;

        bool last = directory[directory_length] == '\0';

        char path[PATH_MAX] = {};
        memcpy(path, directory, directory_length);
        path[directory_length] = '/';

        if (stat(path, &path_stat) == -1)
            continue;

        if (!S_ISDIR(path_stat.st_mode))
            continue;

        DIR *dir = opendir(path);
        struct dirent *entity;
        while ((entity = readdir(dir)) != NULL)
        {
            if (strncmp(prefix, entity->d_name, prefix_length) != 0)
                continue;

            memcpy(path, directory, directory_length);
            path[directory_length] = '/';
            path[directory_length + 1] = '\0';
            strcat(path + directory_length + 1, entity->d_name);

            if (stat(path, &path_stat) == -1)
                continue;

            if (!S_ISREG(path_stat.st_mode))
                continue;

            if (access(path, F_OK | X_OK) != 0)
                continue;

            char *candidate = entity->d_name + prefix_length;
            if (vector_contains(candidates, &candidate, string_compare))
                continue;

            candidate = strdup(candidate);
            vector_append(candidates, &candidate);
        }

        closedir(dir);

        if (last)
            break;
    }
}

static void collect_files(vector_t *candidates, const char *directory, const char *prefix)
{
    size_t prefix_length = strlen(prefix);

    if (!directory)
        directory = ".";

    DIR *dir = opendir(directory);
    struct dirent *entity;
    while ((entity = readdir(dir)) != NULL)
    {
        if (strcmp(entity->d_name, ".") == 0 || strcmp(entity->d_name, "..") == 0)
            continue;

        if (strncmp(prefix, entity->d_name, prefix_length) != 0)
            continue;

        char *candidate = entity->d_name + prefix_length;
        if (entity->d_type == DT_DIR)
        {
            size_t candidate_length = strlen(candidate);
            char *candidate_with_slash = malloc(candidate_length + 2);
            memcpy(candidate_with_slash, candidate, candidate_length);
            candidate_with_slash[candidate_length] = '/';
            candidate_with_slash[candidate_length + 1] = '\0';
            candidate = candidate_with_slash;
        }
        else
            candidate = strdup(candidate);

        if (vector_contains(candidates, &candidate, string_compare))
        {
            free(candidate);
            continue;
        }

        vector_append(candidates, &candidate);
    }

    closedir(dir);
}

static char *find_shared_prefix(vector_t *candidates)
{
    const char *first = *((char **)vector_get(candidates, 0));

    size_t first_length = strlen(first);
    if (first_length == 0)
        return (NULL);

    size_t end = 0;
    while (end < first_length)
    {
        ++end;

        bool one_is_not_matching = false;

        for (size_t index = 1; index < candidates->length; ++index)
        {
            char *other = *((char **)vector_get(candidates, index));

            if (strncmp(first, other, end) != 0)
            {
                one_is_not_matching = true;
                break;
            }
        }

        if (one_is_not_matching)
        {
            --end;
            break;
        }
    }

    if (end == 0)
        return (NULL);

    char *prefix = malloc(end + 1);
    memcpy(prefix, first, end);
    prefix[end] = '\0';

    return (prefix);
}

e_autocomplete_result autocomplete(vector_t *line, bool bell_rung)
{
    vector_t candidates = vector_initialize(sizeof(char *));

    static char SPACE = ' ';
    size_t last_space_index = vector_last_index_of(line, &SPACE, char_compare);
    bool is_command = last_space_index == (size_t)-1;
    char *beginning = is_command
                          ? strndup(line->pointer, line->length)
                          : strndup((char *)line->pointer + last_space_index + 1, line->length - last_space_index - 1);

    char *parent = NULL;
    char *prefix = NULL;

    size_t beginning_length = strlen(beginning);
    if (beginning_length == 0)
    {
        parent = NULL;
        prefix = strdup("");
    }
    else if (beginning[beginning_length - 1] == '/')
    {
        parent = strdup(beginning);
        prefix = strdup("");
    }
    else
    {
        char *last_slash = strrchr(beginning, '/');

        if (last_slash)
        {
            size_t parent_length = last_slash - beginning + 1;
            parent = strndup(beginning, parent_length);
            prefix = strdup(beginning + parent_length);
        }
        else
        {
            parent = NULL;
            prefix = strdup(beginning);
        }
    }

    if (is_command)
    {
        collect_builtins(&candidates, prefix);
        collect_executables(&candidates, prefix);
    }

    if (true)
    {
        collect_files(&candidates, parent, prefix);
    }

    vector_sort(&candidates, string_compare_short_first);

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
        commit(line, *candidate, false);
    }
    else
    {
        char *shared_prefix = find_shared_prefix(&candidates);
        if (shared_prefix)
        {
            result = AR_FOUND;

            commit(line, shared_prefix, true);

            free(shared_prefix);
        }
        else
        {
            result = AR_MORE;

            if (bell_rung)
            {
                write_string("\n");

                for (size_t index = 0; index < candidates.length; ++index)
                {
                    if (index != 0)
                        write_string("  ");

                    write_string(prefix);
                    char **candidate = vector_get(&candidates, index);
                    write_string(*candidate);
                }

                write_string("\n");
                shell_prompt();
                write_string_n(line->pointer, line->length);
            }
        }
    }

    for (size_t index = 0; index < candidates.length; ++index)
        free(*((char **)vector_get(&candidates, index)));

    vector_destroy(&candidates);
    free(beginning);
    free(parent);
    free(prefix);

    return (result);
}
