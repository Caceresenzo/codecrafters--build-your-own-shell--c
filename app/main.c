#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main()
{
	printf("$ ");
	fflush(stdout);

	char input[100];
	fgets(input, 100, stdin);

	
	for (char *ptr = input; *ptr; ++ptr)
	{
		if (isspace(*ptr))
		{
			*ptr = '\0';
			break;
		}
	}
	
	char *program = input;
	printf("%s: command not found\n", program);

	return (EXIT_SUCCESS);
}
