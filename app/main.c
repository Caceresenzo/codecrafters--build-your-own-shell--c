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

void shell_exec(char **argv, int argc, io_t io)
{
	char *program = argv[0];

	builtin_t builtin = builtin_find(program);
	if (builtin)
	{
		builtin(argc, argv, io);
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
			dup2(io.output, STDOUT_FILENO);
			dup2(io.error, STDERR_FILENO);

			io_close(&io);

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
	parsed_line_t parsed_line = line_parse(line);

	io_t io = io_open(parsed_line.redirects, parsed_line.redirect_count);
	if (io.valid)
		shell_exec(parsed_line.argv, parsed_line.argc, io);

	io_close(&io);
	line_free(&parsed_line);
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
