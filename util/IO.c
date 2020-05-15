#include "IO.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void io_print_file(struct IO *const io, const char *const format, va_list args) {
	vfprintf(io->file, format, args);
}

void io_print_string(struct IO *const io, const char *const format, va_list args) {
	va_list args_copy;
	va_copy(args_copy, args);
	size_t len = vsnprintf(NULL, 0, format, args);
	va_end(args_copy);
	io->len += len;
	io->string = (char *) realloc(io->string, io->len + 1);
	vsprintf(io->string + io->len - len, format, args_copy);
}
