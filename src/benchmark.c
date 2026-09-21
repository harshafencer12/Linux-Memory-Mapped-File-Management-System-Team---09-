#define _POSIX_C_SOURCE 200809L

#include "benchmark.h"
#include "proc_stats.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

static double elapsed_time(struct timespec *start,
                           struct timespec *end)
{
    return (double)(end->tv_sec - start->tv_sec)
           + (double)(end->tv_nsec - start->tv_nsec)
             / 1000000000.0;
}

/*
 * Create a test file containing predictable data.
 */
int create_test_file(const char *filename,
                     size_t size)
{
    int fd = open(filename,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0666);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    char buffer[8192];

    for (size_t i = 0; i < sizeof(buffer); i++)
        buffer[i] = (char)('A' + (i % 26));

    size_t written = 0;

    while (written < size)
    {
        size_t remaining = size - written;

        size_t chunk =
            remaining < sizeof(buffer)
            ? remaining
            : sizeof(buffer);

        ssize_t result =
            write(fd, buffer, chunk);

        if (result == -1)
        {
            perror("write");
            close(fd);
            return -1;
        }

        written += (size_t)result;
    }

    close(fd);

    return 0;
}

/*
 * Benchmark traditional read().
 *
 * Every byte read is processed through a checksum so that
 * the amount of work is comparable with the mmap benchmark.
 */
static int benchmark_read_write(const char *filename,
                                size_t size,
                                double *time_taken,
                                unsigned long *minor_faults,
                                unsigned long *major_faults)
{
    FaultStats before;
    FaultStats after;

    if (get_fault_stats(&before) == -1)
        return -1;

    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    char buffer[8192];

    unsigned long long checksum = 0;
    size_t total = 0;

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (total < size)
    {
        size_t remaining = size - total;

        size_t chunk =
            remaining < sizeof(buffer)
            ? remaining
            : sizeof(buffer);

        ssize_t bytes =
            read(fd, buffer, chunk);

        if (bytes < 0)
        {
            perror("read");
            close(fd);
            return -1;
        }

        if (bytes == 0)
            break;

        for (ssize_t i = 0; i < bytes; i++)
            checksum += (unsigned char)buffer[i];

        total += (size_t)bytes;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    close(fd);

    if (get_fault_stats(&after) == -1)
        return -1;

    *time_taken =
        elapsed_time(&start, &end);

    *minor_faults =
        after.minor_faults -
        before.minor_faults;

    *major_faults =
        after.major_faults -
        before.major_faults;

    /*
     * Prevent the compiler from removing the checksum work.
     */
    printf("  Checksum      : %llu\n", checksum);

    return 0;
}

/*
 * Benchmark memory-mapped I/O.
 *
 * Every byte in the mapping is processed so that the workload
 * is comparable with the traditional read() benchmark.
 */
static int benchmark_mmap(const char *filename,
                          size_t size,
                          double *time_taken,
                          unsigned long *minor_faults,
                          unsigned long *major_faults)
{
    FaultStats before;
    FaultStats after;

    if (get_fault_stats(&before) == -1)
        return -1;

    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    if (size == 0)
    {
        close(fd);

        *time_taken = 0.0;
        *minor_faults = 0;
        *major_faults = 0;

        return 0;
    }

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    void *mapped =
        mmap(NULL,
             size,
             PROT_READ,
             MAP_PRIVATE,
             fd,
             0);

    if (mapped == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    unsigned char *data =
        (unsigned char *)mapped;

    unsigned long long checksum = 0;

    for (size_t i = 0; i < size; i++)
        checksum += data[i];

    clock_gettime(CLOCK_MONOTONIC, &end);

    munmap(mapped, size);
    close(fd);

    if (get_fault_stats(&after) == -1)
        return -1;

    *time_taken =
        elapsed_time(&start, &end);

    *minor_faults =
        after.minor_faults -
        before.minor_faults;

    *major_faults =
        after.major_faults -
        before.major_faults;

    /*
     * Prevent the compiler from removing the checksum work.
     */
    printf("  Checksum      : %llu\n", checksum);

    return 0;
}

int run_benchmark(const char *filename,
                  const char *results_file)
{
    /*
     * Open results file in append mode.
     */
    FILE *out = fopen(results_file, "a+");

    if (out == NULL)
    {
        perror("results file");
        return -1;
    }

    /*
     * Add CSV header if this is a new/empty file.
     */
    fseek(out, 0, SEEK_END);

    long file_position = ftell(out);

    if (file_position == 0)
    {
        fprintf(out,
                "size_bytes,method,time_sec,throughput_mb_s,"
                "minor_faults,major_faults\n");
    }

    struct stat st;

    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        fclose(out);
        return -1;
    }

    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        fclose(out);
        return -1;
    }

    close(fd);

    size_t size = (size_t)st.st_size;

    if (size == 0)
    {
        fprintf(stderr,
                "Benchmark file is empty.\n");

        fclose(out);
        return -1;
    }

    double read_time;
    double mmap_time;

    unsigned long read_minor;
    unsigned long read_major;

    unsigned long mmap_minor;
    unsigned long mmap_major;

    printf("\n========== PERFORMANCE BENCHMARK ==========\n");
    printf("File size: %zu bytes\n\n", size);

    printf("Traditional read():\n");

    if (benchmark_read_write(filename,
                             size,
                             &read_time,
                             &read_minor,
                             &read_major) == -1)
    {
        fclose(out);
        return -1;
    }

    double read_mb =
        ((double)size / (1024.0 * 1024.0))
        / read_time;

    printf("  Time          : %.6f sec\n", read_time);
    printf("  Throughput    : %.2f MB/s\n", read_mb);
    printf("  Minor faults  : %lu\n", read_minor);
    printf("  Major faults  : %lu\n\n", read_major);

    printf("Memory mapped:\n");

    if (benchmark_mmap(filename,
                       size,
                       &mmap_time,
                       &mmap_minor,
                       &mmap_major) == -1)
    {
        fclose(out);
        return -1;
    }

    double mmap_mb =
        ((double)size / (1024.0 * 1024.0))
        / mmap_time;

    printf("  Time          : %.6f sec\n", mmap_time);
    printf("  Throughput    : %.2f MB/s\n", mmap_mb);
    printf("  Minor faults  : %lu\n", mmap_minor);
    printf("  Major faults  : %lu\n", mmap_major);

    printf("===========================================\n");

    fprintf(out,
            "%zu,read,%.6f,%.2f,%lu,%lu\n",
            size,
            read_time,
            read_mb,
            read_minor,
            read_major);

    fprintf(out,
            "%zu,mmap,%.6f,%.2f,%lu,%lu\n",
            size,
            mmap_time,
            mmap_mb,
            mmap_minor,
            mmap_major);

    fclose(out);

    return 0;
}
