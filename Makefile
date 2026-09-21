CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -Iinclude
LDLIBS = -pthread

TARGET = mmfs

SRC = \
	src/main.c \
	src/mmap_ops.c \
	src/io_ops.c \
	src/proc_stats.c \
	src/benchmark.c \
	src/concurrency.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDLIBS) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
