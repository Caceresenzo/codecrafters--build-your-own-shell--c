#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#include "shell.h"

bool shell_read(char *buffer, size_t buffer_size)
{
	while (true)
	{
		printf("$ ");
		fflush(stdout);

		memset(buffer, '\0', buffer_size);
		fgets(buffer, buffer_size, stdin);

		size_t length = strlen(buffer);
		if (length == 0)
			return (false);

		if (buffer[length - 1] == '\n')
		{
			buffer[--length] = '\0';

			if (length == 0)
				continue;
		}

		break;
	}

	return (true);
}

void shell_eval(char *line)
{
	int argc = 1;
	for (char *ptr = line; *ptr; ++ptr)
	{
		if (isspace(*ptr))
		{
			*ptr = '\0';
			++argc;
		}
	}

	char *argv[argc + 1];
	argv[0] = line;
	argv[argc] = NULL;

	for (int index = 1; index < argc; ++index)
	{
		char *previous = argv[index - 1];
		size_t offset = strlen(previous) + 1 /* null terminator */;

		// printf("previous=`%s`  offset=%ld\n", previous, offset);

		argv[index] = previous + offset;
	}

	// for (int index = 0; index <= argc; ++index)
	// {
	// 	printf("argv[%d] = `%s`\n", index, argv[index]);
	// }

	char *program = line;

	builtin_t builtin = builtin_find(program);
	if (builtin)
	{
		builtin(argc, argv);
		return;
	}

	printf("%s: command not found\n", program);
}

static size_t strlen_or(const char *str, char alternative_end)
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

int main()
{
	char line[100];

	while (true)
	{
		if (!shell_read(line, sizeof(line)))
			break;

		shell_eval(line);
	}

	return (EXIT_SUCCESS);
}
