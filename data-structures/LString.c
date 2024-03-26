#include "LString.h"

size_t LString_len(const struct LString str) {
	return str.len;
}

const char *LString_chars(const struct LString str) {
	return str.str;
}

void LString_init(struct LString *str, char *chars, const size_t len) {
	str->str = chars;
	str->len = len;
}
