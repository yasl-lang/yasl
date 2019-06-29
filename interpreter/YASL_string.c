#include "YASL_string.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

size_t yasl_string_len(const String_t *const str) {
	return (size_t)(str->end - str->start);
}

int64_t yasl_string_cmp(const String_t *const left, const String_t *const right) {
	if (yasl_string_len(left) == yasl_string_len(right)) {
		return memcmp(left->str + left->start, right->str + right->start, yasl_string_len(left));
	} else if (yasl_string_len(left) < yasl_string_len(right)) {
		int64_t tmp = memcmp(left->str + left->start, right->str + right->start, yasl_string_len(left));
		return tmp ? tmp : -1;
	} else {
		int64_t tmp = memcmp(left->str + left->start, right->str + right->start, yasl_string_len(right));
		return tmp ? tmp : 1;
	}
}

char *copy_char_buffer(const size_t size, const char *const ptr) {
	char *tmp = (char *) malloc(size);
	memcpy(tmp, ptr, size);
	return tmp;
}

String_t *str_new_substring(const size_t start, const size_t end, const String_t *const string) {
	String_t *str = (String_t *) malloc(sizeof(String_t));
	str->on_heap = string->on_heap;
	if (str->on_heap) {
		str->str = (char *)malloc(end - start);
		memcpy(str->str, string->str + start, end - start);
		str->start = 0;
		str->end = end - start;
	} else {
		str->start = start;
		str->end = end;
		str->str = string->str;
	}
	str->rc = rc_new();
	return str;
}

String_t *str_new_sized(const size_t base_size, const char *const ptr) {
	String_t *str = (String_t *) malloc(sizeof(String_t));
	str->start = 0;
	str->end = base_size;
	str->str = (char *) ptr;
	str->on_heap = 0;
	str->rc = rc_new();
	return str;
}

String_t* str_new_sized_heap(const size_t start, const size_t end, const char *const mem) {
	String_t *str = (String_t *) malloc(sizeof(String_t));
	str->start = start;
	str->end = end;
	str->str = (char *) mem;
	str->on_heap = 1;
	str->rc = rc_new();
	return str;
}

void str_del_data(String_t *str) {
	if (str->on_heap) free((void *) str->str);
}

void str_del_rc(String_t *str) {
	rc_del(str->rc);
	free(str);
}

void str_del(String_t *str) {
	if (str->on_heap) free((void *) str->str);
	rc_del(str->rc);
	free(str);
}


int64_t str_find_index(const String_t *haystack, const String_t *needle) {
	// TODO: implement non-naive algorithm for string search.
	if (yasl_string_len(haystack) < yasl_string_len(needle)) return -1;
	size_t i = 0;
	const char *hayStr = haystack->str + haystack->start;
	const char *needleStr = needle->str + needle->start;
	while (i <= yasl_string_len(haystack) - yasl_string_len(needle)) {
		if (!memcmp(hayStr + i, needleStr, yasl_string_len(needle))) return i;
		i++;
	}
	return -1;
}
