#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef void (*builtin_t)(int, char **);
typedef struct
{
	const char *name;
	builtin_t function;
} builtin_entry_t;

void builtin_exit(int argc, char **argv)
{
	exit(0);

	(void)argc;
	(void)argv;
}

void builtin_echo(int argc, char **argv)
{
	int last = argc - 1;
	for (int index = 1; index < argc; ++index)
	{
		printf("%s", argv[index]);

		if (index != last)
			printf(" ");
	}
	
	printf("\n");
}

builtin_entry_t builtin_registry[] = {
	{"exit", builtin_exit},
	{"echo", builtin_echo},
	{},
};

builtin_t builtin_find(const char *name)
{
	for (builtin_entry_t *entry = builtin_registry; entry->name; ++entry)
	{
		if (strcmp(entry->name, name) == 0)
			return (entry->function);
	}

	return (NULL);
}

bool read(char *buffer, size_t buffer_size)
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

void eval(char *line)
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

int main()
{
	char line[100];

	while (true)
	{
		if (!read(line, sizeof(line)))
			break;

		eval(line);
	}

	return (EXIT_SUCCESS);
}
