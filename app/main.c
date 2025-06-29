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

void shell_prompt()
{
	write_string("$ ");
}

#define UP 'A'
#define DOWN 'B'

void change_line(vector_t *line, const char *new_line)
{
	size_t length = line->length;

	char *backspaces = malloc(length);
	memset(backspaces, '\b', length);
	char *spaces = malloc(length);
	memset(spaces, ' ', length);

	write(STDOUT_FILENO, backspaces, length);
	write(STDOUT_FILENO, spaces, length);
	write(STDOUT_FILENO, backspaces, length);

	free(backspaces);
	free(spaces);

	size_t new_length = strlen(new_line);
	write(STDOUT_FILENO, new_line, new_length);

	vector_clear(line);
	vector_add_all_iterate(line, new_line, new_length);
}

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

	size_t history_length = history_size();
	size_t history_position = history_length;

	shell_prompt();

	bool bell_rung = false;

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
		else if (character == '\t')
		{
			e_autocomplete_result autocomplete_result = autocomplete(line, bell_rung);

			if (autocomplete_result == AR_NONE)
			{
				bell();
				bell_rung = false;
			}
			else if (autocomplete_result == AR_FOUND)
				bell_rung = false;
			else if (autocomplete_result == AR_MORE)
			{
				bell();
				bell_rung = true;
			}
		}
		else if (character == 0x1b)
		{
			getchar(); // '['

			char direction = getchar();
			if (direction == UP && history_position != 0)
			{
				history_position--;
				change_line(line, history_get(history_position));
			}
			else if (direction == DOWN && history_position < history_length)
			{
				history_position++;

				if (history_position == history_length)
					change_line(line, "");
				else
					change_line(line, history_get(history_position));
			}
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

command_result_t shell_exec(char **argv, int argc, io_t io)
{
	if (argc == 0)
		return ((command_result_t){
			.exit_code = 0,
			.exit_shell = false,
		});

	char *program = argv[0];

	builtin_t builtin = builtin_find(program);
	if (builtin)
		return (builtin(argc, argv, io));

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

		return ((command_result_t){
			.exit_code = 0,
			.exit_shell = false,
		});
	}

	printf("%s: command not found\n", program);
	return ((command_result_t){
		.exit_code = 127,
		.exit_shell = false,
	});
}

command_result_t shell_eval(char *line)
{
	history_add(line);

	vector_t commands = line_parse(line);

	if (commands.length == 1)
	{
		parsed_line_t *parsed_line = vector_get(&commands, 0);
		command_result_t result = {
			.exit_code = 1,
			.exit_shell = false,
		};

		io_t io = io_open(parsed_line->redirects, parsed_line->redirect_count);
		if (io.valid)
			result = shell_exec(parsed_line->argv, parsed_line->argc, io);

		io_close(&io);
		line_free(parsed_line);
		vector_destroy(&commands);

		return (result);
	}
	else
	{
		pipeline(commands);
		vector_destroy(&commands);

		return ((command_result_t){
			.exit_code = 0,
			.exit_shell = false,
		});
	}
}

int main()
{
	history_initialize();

	vector_t line = vector_initialize(sizeof(char));
	vector_resize(&line, 100);

	int shell_exit_code = EXIT_SUCCESS;

	while (true)
	{
		e_shell_read_result read_result = shell_read(&line);

		if (read_result == SRR_QUIT)
			break;

		if (read_result == SRR_EMPTY)
			continue;

		command_result_t command_result = shell_eval(line.pointer);
		if (command_result.exit_shell)
		{
			shell_exit_code = command_result.exit_code;
			break;
		}
	}

	vector_destroy(&line);

	history_destroy();
	return (shell_exit_code);
}
