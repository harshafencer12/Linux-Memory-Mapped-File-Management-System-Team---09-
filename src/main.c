#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mmap_ops.h"
#include "io_ops.h"
#include "proc_stats.h"
#include "benchmark.h"
#include "concurrency.h"

static void print_usage(const char *program)
{
    printf("\nLinux Memory-Mapped File Management System\n\n");

    printf("Usage:\n");
    printf("  %s map-read <file>\n", program);
    printf("  %s map-write <file> <text>\n", program);
    printf("  %s private <file>\n", program);
    printf("  %s shared <file>\n", program);
    printf("  %s io-read <file>\n", program);
    printf("  %s io-write <file> <text>\n", program);
    printf("  %s inspect\n", program);
    printf("  %s benchmark <file> <results.csv>\n", program);
    printf("  %s create <file> <size>\n", program);
    printf("  %s race <file>\n", program);
    printf("  %s sync <file>\n", program);
    printf("  %s copy-mmap <source> <destination>\n", program);
    printf("  %s copy-io <source> <destination>\n", program);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "map-read") == 0)
    {
        if (argc != 3)
        {
            print_usage(argv[0]);
            return 1;
        }

        return mmap_read_file(argv[2]);
    }

    if (strcmp(argv[1], "map-write") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return 1;
        }

        return mmap_write_file(argv[2],
                               argv[3]);
    }

    if (strcmp(argv[1], "private") == 0)
    {
        if (argc != 3)
        {
            print_usage(argv[0]);
            return 1;
        }

        return mmap_private_demo(argv[2]);
    }

    if (strcmp(argv[1], "shared") == 0)
    {
        if (argc != 3)
        {
            print_usage(argv[0]);
            return 1;
        }

        return mmap_shared_demo(argv[2]);
    }

    if (strcmp(argv[1], "io-read") == 0)
    {
        if (argc != 3)
        {
            print_usage(argv[0]);
            return 1;
        }

        return traditional_read_file(argv[2]);
    }

    if (strcmp(argv[1], "io-write") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return 1;
        }

        return traditional_write_file(argv[2],
                                      argv[3]);
    }

    if (strcmp(argv[1], "inspect") == 0)
    {
        FaultStats stats;

        if (get_fault_stats(&stats) == 0)
        {
            printf("\n========== PROCESS STATISTICS ==========\n");
            printf("PID           : %d\n", getpid());
            printf("Minor faults  : %lu\n",
                   stats.minor_faults);
            printf("Major faults  : %lu\n",
                   stats.major_faults);
        }

        print_memory_maps();

        return 0;
    }

    if (strcmp(argv[1], "benchmark") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return 1;
        }

        return run_benchmark(argv[2],
                             argv[3]);
    }

    if (strcmp(argv[1], "create") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return 1;
        }

        size_t size =
            strtoull(argv[3], NULL, 10);

        printf("Creating %zu byte test file...\n",
               size);

        return create_test_file(argv[2],
                                size);
    }

    if (strcmp(argv[1], "race") == 0)
    {
        if (argc != 3)
        {
            print_usage(argv[0]);
            return 1;
        }

        return run_race_demo(argv[2]);
    }

    if (strcmp(argv[1], "sync") == 0)
    {
        if (argc != 3)
        {
            print_usage(argv[0]);
            return 1;
        }

        return run_sync_demo(argv[2]);
    }

    if (strcmp(argv[1], "copy-mmap") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return 1;
        }

        return mmap_copy_file(argv[2],
                              argv[3]);
    }

    if (strcmp(argv[1], "copy-io") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return 1;
        }

        return traditional_copy_file(argv[2],
                                     argv[3]);
    }

    print_usage(argv[0]);

    return 1;
}
