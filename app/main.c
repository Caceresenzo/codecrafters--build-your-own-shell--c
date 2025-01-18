#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>

#include "shell.h"
#include "vector.h"

typedef enum
{
	SRR_UNSET,
	SRR_QUIT,
	SRR_EMPTY,
	SRR_CONTENT,
} e_shell_read_result;

e_shell_read_result shell_read(vector_t *line)
{
	vector_clear(line);

	struct termios previous;
	tcgetattr(STDIN_FILENO, &previous);

	struct termios new = previous;
	new.c_lflag &= ~(ECHO | ICANON);
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &new);

	printf("$ ");
	fflush(stdout);

	e_shell_read_result result = SRR_UNSET;

	while (result == SRR_UNSET)
	{
		int character_value = getchar();
		if (character_value == -1)
		{
			perror("getchar");
			result = SRR_EMPTY;
			break;
		}

		char character = character_value;

		if (character == 0x4)
		{
			if (vector_is_empty(line))
			{
				result = SRR_QUIT;
				break;
			}
		}
		else if (character == '\n')
		{
			write(STDOUT_FILENO, "\n", 1);
			result = vector_is_empty(line) ? SRR_EMPTY : SRR_CONTENT;
			break;
		}
		else if (character == 0x1b)
		{
			getchar(); // '['
			getchar(); // 'A' or 'B' or 'C' or 'D'
		}
		else if (character == 0x7f)
		{
			if (vector_is_empty(line))
				continue;

			write(STDOUT_FILENO, "\b \b", 3);
			vector_pop(line);
		}
		else
		{
			write(STDOUT_FILENO, &character, 1);
			vector_append(line, &character);
		}
	}

	int zero = 0;
	vector_append(line, &zero);

	tcsetattr(STDIN_FILENO, TCSANOW, &previous);
	return (result);
}

void shell_exec(char **argv, int argc, io_t io)
{
	if (argc == 0)
		return;

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
	vector_t line = vector_initialize(sizeof(char));
	vector_resize(&line, 100);

	while (true)
	{
		e_shell_read_result result = shell_read(&line);

		if (result == SRR_QUIT)
			break;

		if (result == SRR_EMPTY)
			continue;

		shell_eval(line.pointer);
	}

	vector_destroy(&line);
	return (EXIT_SUCCESS);
}
