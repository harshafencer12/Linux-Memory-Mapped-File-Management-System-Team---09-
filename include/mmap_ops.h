#ifndef MMAP_OPS_H
#define MMAP_OPS_H

#include <stddef.h>

int mmap_read_file(const char *filename);
int mmap_write_file(const char *filename, const char *text);
int mmap_private_demo(const char *filename);
int mmap_shared_demo(const char *filename);
int mmap_copy_file(const char *source, const char *destination);

#endif
