#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>

#include "shell.h"
#include "vector.h"

static void _exec(parsed_line_t *command)
{
    io_t io = io_open(command->redirects, command->redirect_count);
    if (!io.valid)
    {
        perror("shell:io_open");
        return (_exit(1));
    }

    if (command->argc == 0)
    {
        io_close(&io);
        return (_exit(0));
    }

    char *program = command->argv[0];

    builtin_t builtin = builtin_find(program);
    if (builtin)
    {
        command_result_t result = builtin(command->argc, command->argv, io);
        io_close(&io);
        return (_exit(result.exit_code));
    }

    char path[PATH_MAX] = {};
    if (!locate(program, path))
    {
        printf("%s: command not found\n", program);
        io_close(&io);
        return (_exit(0));
    }

    dup2(io.output, STDOUT_FILENO);
    dup2(io.error, STDERR_FILENO);

    io_close(&io);

    execvp(path, command->argv);
    perror("execvp");
    _exit(1);
}

static pid_t spawn(int fd_in, int fd_out, parsed_line_t *command)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return (-1);
    }

    if (pid == 0)
    {
        dup2(fd_in, STDIN_FILENO);
        dup2(fd_out, STDOUT_FILENO);

        _exec(command);
    }

    return (pid);
}

pid_t pipeline(vector_t commands)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return (-1);
    }
    else if (pid == 0)
    {
        int fd_in = STDIN_FILENO;

        for (size_t index = 0; index < commands.length - 1; ++index)
        {
            parsed_line_t *parsed_line = vector_get(&commands, index);

            int pipe_fds[2];
            pipe(pipe_fds);

            spawn(fd_in, pipe_fds[1], parsed_line);

            close(pipe_fds[1]);

            fd_in = pipe_fds[0];
        }

        dup2(fd_in, STDIN_FILENO);

        parsed_line_t *parsed_line = vector_get(&commands, commands.length - 1);
        _exec(parsed_line);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    return (pid);
}
