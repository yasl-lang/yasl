#pragma once

#include <stdio.h>

struct IO {
	void (*print)(struct IO *, const char *const, const size_t);
	FILE *file;
	char *string;
	size_t len;
};

void io_print_file(struct IO *io, const char *const format, const size_t len);
void io_print_string(struct IO *io, const char *const format, const size_t len);
