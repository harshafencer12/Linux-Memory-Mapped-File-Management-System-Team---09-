#define _POSIX_C_SOURCE 200809L

#include "mmap_ops.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

static int ensure_file_size(const char *filename, size_t size)
{
    int fd = open(filename, O_RDWR | O_CREAT, 0666);

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

int mmap_read_file(const char *filename)
{
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    struct stat st;

    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        return -1;
    }

    if (st.st_size == 0)
    {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    size_t size = (size_t)st.st_size;

    void *mapped = mmap(NULL,
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

    printf("\n===== MEMORY-MAPPED READ =====\n");
    printf("File: %s\n", filename);
    printf("Size: %zu bytes\n", size);
    printf("Mapped address: %p\n", mapped);
    printf("\nContent:\n");

    fwrite(mapped, 1, size, stdout);

    printf("\n==============================\n");

    munmap(mapped, size);
    close(fd);

    return 0;
}

int mmap_write_file(const char *filename, const char *text)
{
    size_t size = strlen(text);

    int fd = ensure_file_size(filename, size);

    if (fd == -1)
        return -1;

    void *mapped = mmap(NULL,
                        size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fd,
                        0);

    if (mapped == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    memcpy(mapped, text, size);

    if (msync(mapped, size, MS_SYNC) == -1)
    {
        perror("msync");
    }

    printf("Memory-mapped write completed.\n");
    printf("Mapped address: %p\n", mapped);
    printf("Bytes written: %zu\n", size);

    munmap(mapped, size);
    close(fd);

    return 0;
}

int mmap_private_demo(const char *filename)
{
    int fd = open(filename, O_RDWR);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    struct stat st;

    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        return -1;
    }

    if (st.st_size == 0)
    {
        close(fd);
        return 0;
    }

    size_t size = (size_t)st.st_size;

    char *mapped = mmap(NULL,
                        size,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE,
                        fd,
                        0);

    if (mapped == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    printf("\n===== MAP_PRIVATE DEMO =====\n");
    printf("Original first byte: %c\n", mapped[0]);

    mapped[0] = 'X';

    printf("Modified mapped byte: %c\n", mapped[0]);

    msync(mapped, size, MS_SYNC);

    munmap(mapped, size);
    close(fd);

    printf("MAP_PRIVATE uses copy-on-write.\n");
    printf("Changes are not written back to the original file.\n");

    return 0;
}

int mmap_shared_demo(const char *filename)
{
    int fd = open(filename, O_RDWR);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    struct stat st;

    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        return -1;
    }

    if (st.st_size == 0)
    {
        close(fd);
        return 0;
    }

    size_t size = (size_t)st.st_size;

    char *mapped = mmap(NULL,
                        size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fd,
                        0);

    if (mapped == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    printf("\n===== MAP_SHARED DEMO =====\n");
    printf("Mapped address: %p\n", mapped);

    mapped[0] = 'S';

    if (msync(mapped, size, MS_SYNC) == -1)
        perror("msync");

    printf("Modified first byte using MAP_SHARED.\n");
    printf("Change synchronized using msync().\n");

    munmap(mapped, size);
    close(fd);

    return 0;
}

int mmap_copy_file(const char *source, const char *destination)
{
    int src = open(source, O_RDONLY);

    if (src == -1)
    {
        perror("source open");
        return -1;
    }

    struct stat st;

    if (fstat(src, &st) == -1)
    {
        perror("fstat");
        close(src);
        return -1;
    }

    int dst = open(destination,
                    O_RDWR | O_CREAT | O_TRUNC,
                    0666);

    if (dst == -1)
    {
        perror("destination open");
        close(src);
        return -1;
    }

    if (ftruncate(dst, st.st_size) == -1)
    {
        perror("ftruncate");
        close(src);
        close(dst);
        return -1;
    }

    if (st.st_size == 0)
    {
        close(src);
        close(dst);
        return 0;
    }

    size_t size = (size_t)st.st_size;

    void *src_map = mmap(NULL,
                         size,
                         PROT_READ,
                         MAP_PRIVATE,
                         src,
                         0);

    void *dst_map = mmap(NULL,
                         size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED,
                         dst,
                         0);

    if (src_map == MAP_FAILED ||
        dst_map == MAP_FAILED)
    {
        perror("mmap");
        close(src);
        close(dst);
        return -1;
    }

    memcpy(dst_map, src_map, size);

    msync(dst_map, size, MS_SYNC);

    munmap(src_map, size);
    munmap(dst_map, size);

    close(src);
    close(dst);

    return 0;
}
