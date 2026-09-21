#ifndef IO_OPS_H
#define IO_OPS_H

#include <stddef.h>

int traditional_read_file(const char *filename);
int traditional_write_file(const char *filename, const char *text);
int traditional_copy_file(const char *source, const char *destination);

#endif
