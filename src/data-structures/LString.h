#ifndef YASL_LSTRING_H_
#define YASL_LSTRING_H_

#include <stdlib.h>
/*
 * A string with associated length. Used internally in YASL.
 */
struct LString {
	char *str;
	size_t len;
};

size_t LString_len(const struct LString str);
const char *LString_chars(const struct LString str);
void LString_init(struct LString *str, char *chars, const size_t len);
#endif
