#ifndef YASL_YASL_STRING_H_
#define YASL_YASL_STRING_H_

#include "refcount.h"
#include "yasl_conf.h"
#include "yasl_include.h"

struct YASL_List;

struct YASL_String {
	struct RC rc;      // NOTE: RC MUST BE THE FIRST MEMBER OF THIS STRUCT. DO NOT REARRANGE.
	char *str;
	size_t start;
	size_t end;
	bool on_heap;
};

size_t YASL_String_len(const struct YASL_String *const str);
const char *YASL_String_chars(const struct YASL_String *const str);
int64_t YASL_String_cmp(const struct YASL_String *const left, const struct YASL_String *const right);
char *copy_char_buffer(const size_t size, const char *const ptr);
struct YASL_String* YASL_String_new_sized(const size_t base_size, const char *const ptr);
struct YASL_String *YASL_String_new_substring(const size_t start, const size_t end,
					      const struct YASL_String *const string);
struct YASL_String* YASL_String_new_sized_heap(const size_t start, const size_t end, const char *const mem);
void str_del_data(struct YASL_String *const str);
void str_del_rc(struct YASL_String *const str);
void str_del(struct YASL_String *const str);
int64_t str_find_index(const struct YASL_String *const haystack, const struct YASL_String *const needle);

yasl_float YASL_String_tofloat(struct YASL_String *str);
yasl_int YASL_String_toint(struct YASL_String *str);
struct YASL_String *YASL_String_toupper(struct YASL_String *a);
struct YASL_String *YASL_String_tolower(struct YASL_String *a);
bool YASL_String_isalnum(struct YASL_String *a);
bool YASL_String_isal(struct YASL_String *a);
bool YASL_String_isnum(struct YASL_String *a);
bool YASL_String_isspace(struct YASL_String *a);
bool YASL_String_startswith(struct YASL_String *haystack, struct YASL_String *needle);
bool YASL_String_endswith(struct YASL_String *haystack, struct YASL_String *needle);
// Caller makes sure search_str is at least length 1.
struct YASL_String *YASL_String_replace_fast_default(struct YASL_String *str, struct YASL_String *search_str,
					     struct YASL_String *replace_str);
struct YASL_String *YASL_String_replace_fast(struct YASL_String *str, struct YASL_String *search_str,
					     struct YASL_String *replace_str, yasl_int);
yasl_int YASL_String_count(struct YASL_String *haystack, struct YASL_String *needle);
void YASL_String_split_default(struct YASL_List *data, struct YASL_String *haystack);
// Caller makes sure needle is not 0 length
void YASL_String_split_fast(struct YASL_List *data, struct YASL_String *haystack, struct YASL_String *needle);
struct YASL_String *YASL_String_ltrim_default(struct YASL_String *haystack);
struct YASL_String *YASL_String_ltrim(struct YASL_String *haystack, struct YASL_String *needle);
struct YASL_String *YASL_String_rtrim_default(struct YASL_String *haystack);
struct YASL_String *YASL_String_rtrim(struct YASL_String *haystack, struct YASL_String *needle);
struct YASL_String *YASL_String_trim_default(struct YASL_String *haystack);
struct YASL_String *YASL_String_trim(struct YASL_String *haystack, struct YASL_String *needle);
// Caller ensures num is greater than or equal to zero
struct YASL_String *YASL_String_rep_fast(struct YASL_String *string, yasl_int num);

#endif
