#include "IO.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void io_print_none(struct IO *const io, const char *const format, va_list args)
{
	(void)io;
	(void)format;
	(void)args;
}

/**
 * To avoid mucking about with the build process
 */
void nop(void *val)
{
	asm("nop"::"r"(val));
}

void io_print_file(struct IO *const io, const char *const format, va_list args)
{
	vfprintf(io->file, format, args);
}

void io_print_string(struct IO *const io, const char *const format, va_list args)
{
	va_list args_copy;
	va_copy(args_copy, args);
	size_t len = vsnprintf(NULL, 0, format, args_copy);
	va_end(args_copy);
	io->len += len;
	io->string = (char *)realloc(io->string, io->len + 1);
	vsprintf(io->string + io->len - len, format, args);
}

inline size_t io_str_strip_char(char *dest, const char *src, size_t n, char rem)
{
	char *swap = malloc(n);
	if (swap == NULL)
		return -1;
	size_t ct = 0;
	size_t src_caret = 0;
	size_t dest_caret = 0;
	for (size_t i = 0; i < n; i++) {
		/**
		 * slide a cursor and if the char at the cursor equals rem skip the src_caret one position and skip
		 */
		if ((char)src[i] == rem) {
			src_caret++;
			continue;
		}
		/**
		 * Copy from src to dest bytewise
		 */
		dest[dest_caret] = src[src_caret];
		src_caret++;
		dest_caret++;
		ct++;
	}
	/**
	 * zero out the rest of the string to avoid garbage being left in the vm string
	 */
	memset(&dest[ct], 0, n - ct);
	return ct;
}
