#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"

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

void builtin_type(int argc, char **argv)
{
	char *program = argv[1];

	builtin_t builtin = builtin_find(program);
	if (builtin)
	{
		printf("%s: is a shell builtin\n", program);
		return;
	}

	char path[PATH_MAX] = {};
    if (locate(program, path))
    {
	    printf("%s is %s\n", program, path);
		return;
    }

	printf("%s not found\n", program);
}

builtin_entry_t builtin_registry[] = {
	{"exit", builtin_exit},
	{"echo", builtin_echo},
	{"type", builtin_type},
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