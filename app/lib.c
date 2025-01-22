#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>

#include "shell.h"

size_t strlen_or(const char *str, char alternative_end)
{
	const char *start = str;

	while (*str && *str != alternative_end)
		++str;

	return (str - start);
}

bool locate(const char *program, char output[static PATH_MAX])
{
	const char *paths = getenv("PATH");
	if (!paths)
		return (false);

	const char *directory = paths;
	while (true)
	{
		size_t length = strlen_or(directory, ':');
		bool last = directory[length] == '\0';

		char path[PATH_MAX] = {};
		memcpy(path, directory, length);
		path[length] = '/';
		strcat(path + length + 1, program);

		if (access(path, F_OK | X_OK) == 0)
		{
			strcpy(output, path);
			return (true);
		}

		if (last)
			break;

		directory += length + 1;
	}

	return (false);
}

void write_string(const char *string)
{
	write_string_n(string, strlen(string));
}

void write_string_n(const char *string, size_t length)
{
	write(STDOUT_FILENO, string, length);
}
