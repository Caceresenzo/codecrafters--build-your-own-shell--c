#include "shell.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef struct
{
    int number;
    pid_t pid;
    char *command;
} job_t;

static vector_t g_jobs;

void jobs_initialize(void)
{
    g_jobs = vector_initialize(sizeof(job_t));
}

int jobs_get_next_number()
{
    if (vector_is_empty(&g_jobs))
        return (1);

    job_t *last_job = vector_get(&g_jobs, g_jobs.length - 1);
    return (last_job->number + 1);
}

void jobs_add(pid_t pid, char **argv, size_t argc)
{
    int number = jobs_get_next_number();
    char *command = strjoin(argv, argc, " ");

    job_t job = {
        .number = number,
        .pid = pid,
        .command = command};

    vector_append(&g_jobs, &job);

    printf("[%d] %d\n", number, pid);
}

static bool is_job_exited(job_t *job)
{
    int status = 0;
    pid_t pid = waitpid(job->pid, &status, WNOHANG);

    if (pid == 0)
        return (false);

    return (WIFEXITED(status) || WIFSIGNALED(status));
}

void jobs_dump()
{
    size_t most_recent_index = g_jobs.length - 1;
    size_t previous_index = most_recent_index - 1;

    vector_t indices_to_remove = vector_initialize(sizeof(size_t));

    for (size_t index = 0; index < g_jobs.length; ++index)
    {
        job_t *job = vector_get(&g_jobs, index);

        char symbol = ' ';
        if (index == most_recent_index)
            symbol = '+';
        else if (index == previous_index)
            symbol = '-';
        
        bool exited = is_job_exited(job);

        const char *status = "Running";
        if (exited)
        {
            status = "Done";
            vector_append(&indices_to_remove, &index);
        }

        printf("[%d]%c  %-20s %s &\n", job->number, symbol, status, job->command);
    }

    for (size_t index = indices_to_remove.length - 1; index != (size_t)-1; --index)
    {
        size_t *job_index = vector_get(&indices_to_remove, index);
        job_t *job = vector_get(&g_jobs, *job_index);
        free(job->command);
        vector_remove(&g_jobs, *job_index);
    }

    vector_destroy(&indices_to_remove);
}

void jobs_destroy(void)
{
    for (size_t index = 0; index < g_jobs.length; ++index)
    {
        job_t *job = vector_get(&g_jobs, index);
        free(job->command);
    }

    vector_destroy(&g_jobs);
}
