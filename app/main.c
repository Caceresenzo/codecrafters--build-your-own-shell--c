#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>

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

void shell_exec(char **argv, int argc)
{
	char *program = argv[0];

	builtin_t builtin = builtin_find(program);
	if (builtin)
	{
		builtin(argc, argv);
		return;
	}

	char path[PATH_MAX] = {};
	if (locate(program, path))
	{
		pid_t pid = fork();
		if (pid == -1)
			perror("fork");
		else if (pid == 0)
		{
			execvp(path, argv);
			perror("execvp");
			exit(1);
		}
		else
			waitpid(pid, NULL, 0);

		return;
	}

	printf("%s: command not found\n", program);
}

void shell_eval(char *line)
{
	char **argv = line_parse(line);
	int argc;

	for (argc = 0; argv[argc]; ++argc)
		;

	// for (int index = 0; index <= argc; ++index)
	// {
	// 	printf("argv[%d] = `%s`\n", index, argv[index]);
	// }

	shell_exec(argv, argc);

	line_free(argv);
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
