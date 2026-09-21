#include "io_ops.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#define BUFFER_SIZE 8192

int traditional_read_file(const char *filename)
{
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    printf("\n===== TRADITIONAL READ =====\n");

    while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
    {
        fwrite(buffer, 1, bytes, stdout);
    }

    if (bytes == -1)
        perror("read");

    close(fd);

    printf("\n============================\n");

    return 0;
}

int traditional_write_file(const char *filename, const char *text)
{
    int fd = open(filename,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0666);

    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    size_t length = strlen(text);
    size_t written = 0;

    while (written < length)
    {
        ssize_t result = write(fd,
                               text + written,
                               length - written);

        if (result == -1)
        {
            perror("write");
            close(fd);
            return -1;
        }

        written += (size_t)result;
    }

    close(fd);

    printf("Traditional write completed: %zu bytes.\n",
           written);

    return 0;
}

int traditional_copy_file(const char *source,
                          const char *destination)
{
    int src = open(source, O_RDONLY);

    if (src == -1)
    {
        perror("source open");
        return -1;
    }

    int dst = open(destination,
                   O_WRONLY | O_CREAT | O_TRUNC,
                   0666);

    if (dst == -1)
    {
        perror("destination open");
        close(src);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    while ((bytes = read(src,
                         buffer,
                         sizeof(buffer))) > 0)
    {
        ssize_t total = 0;

        while (total < bytes)
        {
            ssize_t written =
                write(dst,
                      buffer + total,
                      bytes - total);

            if (written == -1)
            {
                perror("write");
                close(src);
                close(dst);
                return -1;
            }

            total += written;
        }
    }

    if (bytes == -1)
        perror("read");

    close(src);
    close(dst);

    return 0;
}
