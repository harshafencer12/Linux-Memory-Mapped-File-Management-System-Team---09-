#define _POSIX_C_SOURCE 200809L

#include "concurrency.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <semaphore.h>

#define ITERATIONS 100000

typedef struct
{
    int counter;
} SharedData;

typedef struct
{
    SharedData data;
    sem_t semaphore;
} SharedSyncData;

static int prepare_file(const char *filename, size_t size)
{
    int fd = open(filename,
                  O_RDWR | O_CREAT | O_TRUNC,
                  0666);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    if (ftruncate(fd, (off_t)size) == -1)
    {
        perror("ftruncate");
        close(fd);
        return -1;
    }

    return fd;
}

static void race_worker(SharedData *data)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        /*
         * Deliberately split the read-modify-write operation.
         * This creates a larger window in which both processes
         * can read the same counter value.
         */
        int value = data->counter;

        /*
         * Yield the CPU so the other process gets an opportunity
         * to read the same value before this process writes it.
         */
        sched_yield();

        data->counter = value + 1;
    }
}

static void sync_worker(SharedSyncData *data)
{
    for (int i = 0; i < ITERATIONS; i++)
    {
        if (sem_wait(&data->semaphore) == -1)
        {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }

        /*
         * Critical section:
         * read -> modify -> write
         */
        data->data.counter++;

        if (sem_post(&data->semaphore) == -1)
        {
            perror("sem_post");
            exit(EXIT_FAILURE);
        }
    }
}

int run_race_demo(const char *filename)
{
    int fd = prepare_file(filename, sizeof(SharedData));

    if (fd == -1)
        return -1;

    SharedData *data =
        mmap(NULL,
             sizeof(SharedData),
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             fd,
             0);

    if (data == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    data->counter = 0;

    printf("\n========== RACE CONDITION DEMO ==========\n");
    printf("Two processes update the same shared counter.\n");
    printf("No synchronization is used.\n\n");

    pid_t p1 = fork();

    if (p1 == -1)
    {
        perror("fork");
        munmap(data, sizeof(SharedData));
        close(fd);
        return -1;
    }

    if (p1 == 0)
    {
        race_worker(data);
        munmap(data, sizeof(SharedData));
        close(fd);
        _exit(EXIT_SUCCESS);
    }

    pid_t p2 = fork();

    if (p2 == -1)
    {
        perror("fork");
        waitpid(p1, NULL, 0);
        munmap(data, sizeof(SharedData));
        close(fd);
        return -1;
    }

    if (p2 == 0)
    {
        race_worker(data);
        munmap(data, sizeof(SharedData));
        close(fd);
        _exit(EXIT_SUCCESS);
    }

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);

    int expected = ITERATIONS * 2;
    int actual = data->counter;

    printf("Expected counter: %d\n", expected);
    printf("Actual counter:   %d\n", actual);

    if (actual < expected)
    {
        printf("\nRace condition observed:\n");
        printf("Some increments were lost because both processes\n");
        printf("could read and modify the shared value concurrently.\n");
    }
    else
    {
        printf("\nThe final value matched the expected value in this run.\n");
        printf("The program still contains an unsynchronized data race;\n");
        printf("race manifestation can vary between executions.\n");
    }

    munmap(data, sizeof(SharedData));
    close(fd);

    return 0;
}

int run_sync_demo(const char *filename)
{
    size_t total_size = sizeof(SharedSyncData);

    int fd = prepare_file(filename, total_size);

    if (fd == -1)
        return -1;

    SharedSyncData *data =
        mmap(NULL,
             total_size,
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             fd,
             0);

    if (data == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    data->data.counter = 0;

    if (sem_init(&data->semaphore, 1, 1) == -1)
    {
        perror("sem_init");
        munmap(data, total_size);
        close(fd);
        return -1;
    }

    printf("\n========== SYNCHRONIZED DEMO ==========\n");
    printf("Two processes update the same shared counter.\n");
    printf("A process-shared POSIX semaphore protects the critical section.\n\n");

    pid_t p1 = fork();

    if (p1 == -1)
    {
        perror("fork");
        sem_destroy(&data->semaphore);
        munmap(data, total_size);
        close(fd);
        return -1;
    }

    if (p1 == 0)
    {
        sync_worker(data);
        munmap(data, total_size);
        close(fd);
        _exit(EXIT_SUCCESS);
    }

    pid_t p2 = fork();

    if (p2 == -1)
    {
        perror("fork");
        waitpid(p1, NULL, 0);
        sem_destroy(&data->semaphore);
        munmap(data, total_size);
        close(fd);
        return -1;
    }

    if (p2 == 0)
    {
        sync_worker(data);
        munmap(data, total_size);
        close(fd);
        _exit(EXIT_SUCCESS);
    }

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);

    int expected = ITERATIONS * 2;
    int actual = data->data.counter;

    printf("Expected counter: %d\n", expected);
    printf("Actual counter:   %d\n", actual);

    if (actual == expected)
    {
        printf("\nSynchronization successful.\n");
        printf("The semaphore protected the read-modify-write operation.\n");
    }
    else
    {
        printf("\nUnexpected result: synchronization did not produce\n");
        printf("the expected counter value.\n");
    }

    if (sem_destroy(&data->semaphore) == -1)
        perror("sem_destroy");

    munmap(data, total_size);
    close(fd);

    return 0;
}
