#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <yasl_include.h>

#include "interpreter/refcount.h"

typedef struct {
	struct RC *rc;      // NOTE: RC MUST BE THE FIRST MEMBER OF THIS STRUCT. DO NOT REARRANGE.
	char *str;
	size_t start;
	size_t end;
	bool on_heap;
} String_t;


size_t yasl_string_len(const String_t *const str);
int64_t yasl_string_cmp(const String_t *const left, const String_t *const right);
char *copy_char_buffer(const size_t size, const char *const ptr);
String_t* str_new_sized(size_t base_size, const char *const ptr);
String_t *str_new_substring(const size_t start, const size_t end, const String_t *const string);
String_t* str_new_sized_heap(const size_t start, const size_t end, const char *const mem);
void str_del_data(String_t *const str);
void str_del_rc(String_t *const str);
void str_del(String_t *const str);
int64_t str_find_index(const String_t *const haystack, const String_t *const needle);
