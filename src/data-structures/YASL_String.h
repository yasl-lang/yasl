#ifndef YASL_YASL_STRING_H_
#define YASL_YASL_STRING_H_

#include "refcount.h"
#include "yasl_conf.h"
#include "yasl_include.h"
#include "LString.h"

#define iswhitespace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\v' || (c) == '\r')

struct YASL_List;
struct VM;

/*
 * Reference-counted string type. Used in the YASL interpreter.
 */
struct YASL_String {
	struct RC rc;      // NOTE: RC MUST BE THE FIRST MEMBER OF THIS STRUCT. DO NOT REARRANGE.
	struct LString s;
};

size_t YASL_String_len(const struct YASL_String *const str);
const char *YASL_String_chars(const struct YASL_String *const str);

struct YASL_String *YASL_String_new_copy_unbound(const char *const ptr, const size_t size);
#define YASL_String_new_copyz_unbound(ptr) YASL_String_new_copy_unbound((ptr), strlen(ptr))
struct YASL_String* YASL_String_new_take_unbound(const char *const mem, const size_t size);

struct YASL_String *YASL_String_new_copy(struct VM *vm, const char *const ptr, const size_t size);
#define YASL_String_new_copyz(vm, ptr) YASL_String_new_copy((vm), (ptr), strlen(ptr))
struct YASL_String *YASL_String_new_substring(struct VM *vm, const struct YASL_String *const string,
					      const size_t start, const size_t end);
struct YASL_String* YASL_String_new_take(struct VM *vm, const char *const mem, const size_t size);
#define YASL_String_new_takebb(vm, bb) YASL_String_new_take(vm, (char *)(bb)->items, (bb)->count)

void str_del_data(struct YASL_String *const str);
void str_del_rc(struct YASL_String *const str);
void str_del(struct YASL_String *const str);

int64_t str_find_index(const struct YASL_String *const haystack, const struct YASL_String *const needle);
int64_t YASL_String_cmp(const struct YASL_String *const left, const struct YASL_String *const right);
yasl_float YASL_String_tofloat(const char *chars, const size_t len);
yasl_int YASL_String_toint(const char *chars, const size_t len);
struct YASL_String *YASL_String_toupper(struct VM *vm, struct YASL_String *a);
struct YASL_String *YASL_String_tolower(struct VM *vm, struct YASL_String *a);
bool YASL_String_startswith(struct YASL_String *haystack, struct YASL_String *needle);
bool YASL_String_endswith(struct YASL_String *haystack, struct YASL_String *needle);
// Caller makes sure search_str is at least length 1.
struct YASL_String *YASL_String_replace_fast_default(struct VM *vm, struct YASL_String *str, struct YASL_String *search_str,
					     struct YASL_String *replace_str, int *replacements);
struct YASL_String *YASL_String_replace_fast(struct VM *vm, struct YASL_String *str, struct YASL_String *search_str,
					     struct YASL_String *replace_str, int *replacements, yasl_int max_replacements);
yasl_int YASL_String_count(struct YASL_String *haystack, struct YASL_String *needle);
void YASL_String_split_default(struct VM *vm, struct YASL_List *data, struct YASL_String *haystack);
// Caller makes sure max_splits > 0
void YASL_String_split_default_max(struct VM *vm, struct YASL_List *data, struct YASL_String *haystack, yasl_int max_splits);
// Caller makes sure needle is not 0 length
void YASL_String_split_fast(struct VM *vm, struct YASL_List *data, struct YASL_String *haystack, struct YASL_String *needle);
// Caller makes sure needle is not 0 length and max_splits > 0
void YASL_String_split_max_fast(struct VM *vm, struct YASL_List *data, struct YASL_String *haystack, struct YASL_String *needle, yasl_int max_splits);
struct YASL_String *YASL_String_ltrim_default(struct VM *vm, struct YASL_String *haystack);
struct YASL_String *YASL_String_ltrim(struct VM *vm, struct YASL_String *haystack, struct YASL_String *needle);
struct YASL_String *YASL_String_rtrim_default(struct VM *vm, struct YASL_String *haystack);
struct YASL_String *YASL_String_rtrim(struct VM *vm, struct YASL_String *haystack, struct YASL_String *needle);
struct YASL_String *YASL_String_trim_default(struct VM *vm, struct YASL_String *haystack);
struct YASL_String *YASL_String_trim(struct VM *vm, struct YASL_String *haystack, struct YASL_String *needle);
// Caller ensures num is greater than or equal to zero
struct YASL_String *YASL_String_rep_fast(struct VM *vm, struct YASL_String *string, yasl_int num);

#endif
