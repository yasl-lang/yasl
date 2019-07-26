#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <yasl_include.h>
#include "yasl_conf.h"

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
String_t* str_new_sized(const size_t base_size, const char *const ptr);
String_t *str_new_substring(const size_t start, const size_t end, const String_t *const string);
String_t* str_new_sized_heap(const size_t start, const size_t end, const char *const mem);
void str_del_data(String_t *const str);
void str_del_rc(String_t *const str);
void str_del(String_t *const str);
int64_t str_find_index(const String_t *const haystack, const String_t *const needle);

yasl_float string_tofloat(String_t *str);
yasl_int string_toint(String_t *str);
String_t *string_toupper(String_t *a);
String_t *string_tolower(String_t *a);
bool string_isalnum(String_t *a);
bool string_isal(String_t *a);
bool string_isnum(String_t *a);
bool string_isspace(String_t *a);
bool string_startswith(String_t *haystack, String_t *needle);
bool string_endswith(String_t *haystack, String_t *needle);
// Caller makes sure search_str is at least length 1.
String_t *string_replace(String_t *str, String_t *search_str, String_t *replace_str);
yasl_int string_count(String_t *haystack, String_t *needle);
struct RC_UserData *string_split_default(String_t *haystack);
// Caller makes sure needle is not 0 length
struct RC_UserData *string_split(String_t *haystack, String_t *needle);
String_t *string_ltrim_default(String_t *haystack);
String_t *string_ltrim(String_t *haystack, String_t *needle);
String_t *string_rtrim_default(String_t *haystack);
String_t *string_rtrim(String_t *haystack, String_t *needle);
String_t *string_trim_default(String_t *haystack);
String_t *string_trim(String_t *haystack, String_t *needle);
// Caller ensures num is greater than or equal to zero
String_t *string_rep(String_t *string, yasl_int num);
