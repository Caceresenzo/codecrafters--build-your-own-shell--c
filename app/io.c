#include "shell.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

static void do_close(int *fd, int standard_fd)
{
    if (*fd != -1 && *fd != standard_fd)
    {
        close(*fd);
        *fd = standard_fd;
    }
}

io_t io_open(redirect_t *redirects, int redirect_count)
{
    int output = STDOUT_FILENO;
    int error = STDERR_FILENO;

    for (int index = 0; index < redirect_count; ++index)
    {
        redirect_t *redirect = &redirects[index];

        int flags = O_CREAT | O_WRONLY;
        if (redirect->append)
            flags |= O_APPEND;
        else
            flags |= O_TRUNC;

        int fd = open(redirect->path, flags, 0644);
        if (fd == -1)
        {
            dprintf(STDERR_FILENO, "shell: %s: %s", redirect->path, strerror(errno));
            return ((io_t){false, -1, -1});
        }

        if (redirect->file_descriptor == STDOUT_FILENO)
        {
            do_close(&output, STDOUT_FILENO);
            output = fd;
        }
        else if (redirect->file_descriptor == STDERR_FILENO)
        {
            do_close(&error, STDERR_FILENO);
            error = fd;
        }
        else
        {
            close(fd);
        }
    }

    return ((io_t){
        .valid = true,
        .output = output,
        .error = error,
    });
}

void io_close(io_t *io)
{
    do_close(&io->output, STDOUT_FILENO);
    do_close(&io->error, STDERR_FILENO);
}
