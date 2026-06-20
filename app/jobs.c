#include "shell.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


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

void jobs_dump(void)
{
    for (size_t index = 0; index < g_jobs.length; ++index)
    {
        job_t *job = vector_get(&g_jobs, index);
        printf("[%d]+  Running                 %s &\n", job->number, job->command);
    }
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
