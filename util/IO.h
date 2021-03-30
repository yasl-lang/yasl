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
	void (*print)(struct IO *const, const char *const, va_list);
	FILE *file;
	char *string;
	size_t len;
};

void io_print_none(struct IO *const io, const char *const format, va_list args);
void io_print_file(struct IO *const io, const char *const format, va_list);
void io_print_string(struct IO *const io, const char *const format, va_list);
size_t io_str_strip_char(char *dest, const char *src, size_t n, char rem);

#endif
