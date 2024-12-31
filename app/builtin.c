#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"

typedef struct
{
	const char *name;
	builtin_t function;
} builtin_entry_t;

void builtin_exit(int argc, char **argv, io_t io)
{
	exit(0);

	(void)argc;
	(void)argv;
	(void)io;
}

void builtin_echo(int argc, char **argv, io_t io)
{
	int last = argc - 1;
	for (int index = 1; index < argc; ++index)
	{
		dprintf(io.output, "%s", argv[index]);

		if (index != last)
			dprintf(io.output, " ");
	}

	dprintf(io.output, "\n");
}

void builtin_type(int argc, char **argv, io_t io)
{
	char *program = argv[1];

	builtin_t builtin = builtin_find(program);
	if (builtin)
	{
		dprintf(io.output, "%s is a shell builtin\n", program);
		return;
	}

	char path[PATH_MAX] = {};
	if (locate(program, path))
	{
		dprintf(io.output, "%s is %s\n", program, path);
		return;
	}

	dprintf(io.output, "%s: not found\n", program);
}

void builtin_pwd(int argc, char **argv, io_t io)
{
	char path[PATH_MAX] = {};
	getcwd(path, sizeof(path));

	dprintf(io.output, "%s\n", path);
}

void builtin_cd(int argc, char **argv, io_t io)
{
	char absolute_path[PATH_MAX] = {};

	const char *path = argv[1];

	if (path[0] == '/')
		strcpy(absolute_path, path);
	else if (path[0] == '.')
	{
		getcwd(absolute_path, sizeof(absolute_path));
		strcat(absolute_path, "/");
		strcat(absolute_path, path);
	}
	else if (path[0] == '~')
	{
		const char *home = getenv("HOME");
		if (!home)
			printf("cd: $HOME is not set\n");
		else
		{
			strcpy(absolute_path, home);
			strcat(absolute_path, "/");
			strcat(absolute_path, path + 1 /* ~ */);
		}
	}

	if (!*absolute_path)
		return;

	if (chdir(absolute_path) == -1)
		dprintf(io.error, "cd: %s: No such file or directory\n", path);
}

builtin_entry_t builtin_registry[] = {
	{"exit", builtin_exit},
	{"echo", builtin_echo},
	{"type", builtin_type},
	{"pwd", builtin_pwd},
	{"cd", builtin_cd},
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
