#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <yasl_include.h>
#include "yasl_conf.h"

#include "interpreter/refcount.h"

struct YASL_String {
	struct RC *rc;      // NOTE: RC MUST BE THE FIRST MEMBER OF THIS STRUCT. DO NOT REARRANGE.
	char *str;
	size_t start;
	size_t end;
	bool on_heap;
};


size_t yasl_string_len(const struct YASL_String *const str);
int64_t yasl_string_cmp(const struct YASL_String *const left, const struct YASL_String *const right);
char *copy_char_buffer(const size_t size, const char *const ptr);
struct YASL_String* str_new_sized(const size_t base_size, const char *const ptr);
struct YASL_String *str_new_substring(const size_t start, const size_t end, const struct YASL_String *const string);
struct YASL_String* str_new_sized_heap(const size_t start, const size_t end, const char *const mem);
void str_del_data(struct YASL_String *const str);
void str_del_rc(struct YASL_String *const str);
void str_del(struct YASL_String *const str);
int64_t str_find_index(const struct YASL_String *const haystack, const struct YASL_String *const needle);

yasl_float string_tofloat(struct YASL_String *str);
yasl_int string_toint(struct YASL_String *str);
struct YASL_String *string_toupper(struct YASL_String *a);
struct YASL_String *string_tolower(struct YASL_String *a);
bool string_isalnum(struct YASL_String *a);
bool string_isal(struct YASL_String *a);
bool string_isnum(struct YASL_String *a);
bool string_isspace(struct YASL_String *a);
bool string_startswith(struct YASL_String *haystack, struct YASL_String *needle);
bool string_endswith(struct YASL_String *haystack, struct YASL_String *needle);
// Caller makes sure search_str is at least length 1.
struct YASL_String *string_replace(struct YASL_String *str, struct YASL_String *search_str, struct YASL_String *replace_str);
yasl_int string_count(struct YASL_String *haystack, struct YASL_String *needle);
struct RC_UserData *string_split_default(struct YASL_String *haystack);
// Caller makes sure needle is not 0 length
struct RC_UserData *string_split(struct YASL_String *haystack, struct YASL_String *needle);
struct YASL_String *string_ltrim_default(struct YASL_String *haystack);
struct YASL_String *string_ltrim(struct YASL_String *haystack, struct YASL_String *needle);
struct YASL_String *string_rtrim_default(struct YASL_String *haystack);
struct YASL_String *string_rtrim(struct YASL_String *haystack, struct YASL_String *needle);
struct YASL_String *string_trim_default(struct YASL_String *haystack);
struct YASL_String *string_trim(struct YASL_String *haystack, struct YASL_String *needle);
// Caller ensures num is greater than or equal to zero
struct YASL_String *string_rep(struct YASL_String *string, yasl_int num);
