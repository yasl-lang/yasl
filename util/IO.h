#ifndef YASL_IO_H_
#define YASL_IO_H_

#include <stdio.h>

#define NEW_IO(f) ((struct IO) {\
	.print = io_print_file,\
	.file = (f),\
	.string = NULL,\
	.len = 0\
})

struct IO {
	void (*print)(struct IO *const, const char *const, const size_t);
	FILE *file;
	char *string;
	size_t len;
};

void io_print_file(struct IO *const io, const char *const format, const size_t len);
void io_print_string(struct IO *const io, const char *const format, const size_t len);

#endif
