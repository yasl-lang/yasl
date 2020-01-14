#pragma once

#include <stdio.h>

struct IO {
	void (*print)(struct IO *const, const char *const, const size_t);
	FILE *file;
	char *string;
	size_t len;
};

#define NEW_IO() ((struct IO) {\
	.print = io_print_file,\
	.file = stderr,\
	.string = NULL,\
	.len = 0\
	})

void io_print_file(struct IO *const io, const char *const format, const size_t len);
void io_print_string(struct IO *const io, const char *const format, const size_t len);
