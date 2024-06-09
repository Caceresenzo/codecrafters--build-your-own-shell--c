#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

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
	size_t space_count = 0;
	for (char *ptr = line; *ptr; ++ptr)
	{
		if (isspace(*ptr))
		{
			*ptr = '\0';
			++space_count;
		}
	}

	char *program = line;
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
