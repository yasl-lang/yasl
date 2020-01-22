#include "IO.h"

#include <stdlib.h>
#include <string.h>

void io_print_file(struct IO *const io, const char *const format, const size_t len) {
	fwrite(format, 1, len, io->file);
}

void io_print_string(struct IO *const io, const char *const format, const size_t len) {
	size_t new_len = io->len + len;
	io->string = (char *) realloc(io->string, new_len);
	memcpy(io->string + io->len, format, len);
	io->len += len;
}
