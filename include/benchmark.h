#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>

int create_test_file(const char *filename, size_t size);

int run_benchmark(const char *filename, const char *result_file);

#endif
