#include "YASL_String.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "debug.h"
#include "YASL_List.h"
#include "interpreter/YASL_Object.h"
#include "data-structures/YASL_ByteBuffer.h"

struct YASL_String *vm_lookup_interned_str(struct VM *vm, const char *chars, const size_t size);

size_t YASL_String_len(const struct YASL_String *const str) {
	return LString_len(str->s);
}

const char *YASL_String_chars(const struct YASL_String *const str) {
	return LString_chars(str->s);
}

#define CHARS(s) YASL_String_chars(s)
#define LEN(s) YASL_String_len(s)

int64_t YASL_String_cmp(const struct YASL_String *const left, const struct YASL_String *const right) {
	const size_t left_len = YASL_String_len(left);
	const size_t right_len = YASL_String_len(right);
	const char *left_chars = YASL_String_chars(left);
	const char *right_chars = YASL_String_chars(right);
	if (left_len == right_len) {
		return memcmp(left_chars, right_chars, left_len);
	} else if (left_len < right_len) {
		int64_t tmp = memcmp(left_chars, right_chars, left_len);
		return tmp ? tmp : -1;
	} else {
		int64_t tmp = memcmp(left_chars, right_chars, right_len);
		return tmp ? tmp : 1;
	}
}

struct YASL_String *YASL_String_new_substring(struct VM *vm, const struct YASL_String *const string,
					      const size_t start, const size_t end) {
	return YASL_String_new_copy(vm, CHARS(string) + start, end - start);
}

struct YASL_String *YASL_String_new_copy(struct VM *vm, const char *const ptr, const size_t base_size) {
	struct YASL_String *string = vm_lookup_interned_str(vm, ptr, base_size);
	if (string) {
		return string;
	}

	return YASL_String_new_copy_unbound(ptr, base_size);
}

struct YASL_String *YASL_String_new_copy_unbound(const char *const ptr, const size_t base_size) {
	char *const mem = (char *)malloc(base_size + 1);
	memcpy(mem, ptr, base_size);
	mem[base_size] = '\0';
	return YASL_String_new_take_unbound(mem, base_size);
}

struct YASL_String *YASL_String_new_take(struct VM *vm, const char *const mem, const size_t base_size) {
	struct YASL_String *string = vm_lookup_interned_str(vm, mem, base_size);
	if (string) {
		free((char *)mem);
		return string;
	}

	return YASL_String_new_take_unbound(mem, base_size);
}

struct YASL_String *YASL_String_new_take_unbound(const char *const mem, const size_t base_size) {
	struct YASL_String *str = (struct YASL_String *) malloc(sizeof(struct YASL_String));
	LString_init(&str->s, (char *)mem, base_size);
	str->rc = NEW_RC();
	return str;
}

void str_del_data(struct YASL_String *const str) {
	free((void *) CHARS(str));
}

void str_del_rc(struct YASL_String *const str) {
	free(str);
}

void str_del(struct YASL_String *const str) {
	// YASL_ASSERT(CHARS(str)[LEN(str)] == '\0', "expected a nul-terminator");
	str_del_data(str);
	free(str);
}


/********************************************************************************
 *                                                                              *
 *                             String Functions                                 *
 *                                                                              *
 ********************************************************************************/


bool isvaliddouble(const char *str) {
	const size_t len = strlen(str);
	bool hasdot = false;
	bool hase = false;
	for (size_t i = 0; i < len; i++) {
		switch (str[i]) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		case '.':
			if (hase || hasdot) return false;
			hasdot = true;
			break;
		case 'e':
		case 'E':
			if (hase) return false;
			hase = true;
			break;
		case '-':
		case '+':
			if (i > 0 && (str[i-1] == 'e' || str[i-1] == 'E')) break;
			return false;
		default:
			return false;

		}
	}
	return hasdot && isdigit((int)str[len-1]) && isdigit((int)str[0]);
}

int64_t str_find_index(const struct YASL_String *const haystack, const struct YASL_String *const needle) {
	// TODO: implement non-naive algorithm for string search.
	const yasl_int haystack_len = YASL_String_len(haystack);
	const yasl_int needle_len = YASL_String_len(needle);
	if (haystack_len < needle_len) return -1;
	yasl_int i = 0;
	const char *hayStr = YASL_String_chars(haystack);
	const char *needleStr = YASL_String_chars(needle);
	while (i <= haystack_len - needle_len) {
		if (!memcmp(hayStr + i, needleStr, needle_len)) return i;
		i++;
	}
	return -1;
}

static yasl_float parsedouble(const char *str, bool *ok) {
	*ok = true;
	if (!strcmp(str, "inf") || !strcmp(str, "+inf")) return INFINITY;
	else if (!strcmp(str, "-inf")) return -INFINITY;
	else if (str[0] == '-' && isvaliddouble(str+1))
		return -strtod(str+1, NULL);
	else if (str[0] == '+' && isvaliddouble(str+1))
		return +strtod(str+1, NULL);
	else if (isvaliddouble(str))
		return strtod(str, NULL);
	*ok = false;
	return NAN;
}

static yasl_int parseint64(const char *str, bool *ok) {
	yasl_int result;
	size_t len = strlen(str);
	char *end;
	if (len > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		result = strtoll(str + 2, &end, 16);
	} else if (len > 2 && str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
		result = strtoll(str + 2, &end, 2);
	} else {
		result = strtoll(str, &end, 10);
	}
	*ok = str + len == end;
	return str + len == end ? result : 0;
}

yasl_float YASL_String_tofloat(const char *chars, const size_t len) {
	char *buffer = (char *)malloc(len + 1);
	if (!isdigit((int)chars[0])) {
		free(buffer);
		return NAN;
	}
	size_t curr = 0;
	for (size_t i = 0; i < len; ++i) {
		if (chars[i] == '_' && chars[i-1] != '.') {
			continue;
		}
		buffer[curr++] = chars[i];
	}
	buffer[curr] = '\0';
	bool ok = true;
	yasl_float result = parsedouble(buffer, &ok);
	if (ok) {
		free(buffer);
		return result;
	}

	result = (yasl_float)parseint64(buffer, &ok);
	if (ok) {
		free(buffer);
		return result;
	}

	free(buffer);
	return NAN;
}

yasl_int YASL_String_toint(const char *chars, const size_t len) {
	char *buffer = (char *)malloc(len + 1);

	if (len <= 2) {
		memcpy(buffer, chars, len);
		buffer[len] = '\0';
		bool ok;
		yasl_int tmp = parseint64(buffer, &ok);
		YASL_ASSERT(ok, "parsing int from small buffer should never fail.");
		free(buffer);
		return tmp;
	}

	if (chars[0] == '0' && (isalpha((int)chars[1]))) {
		size_t curr = 2;
		buffer[0] = chars[0];
		buffer[1] = chars[1];
		for (size_t i = 2; i < len; ++i) {
			if (chars[i] == '_') {
				continue;
			}
			buffer[curr++] = chars[i];
		}
		buffer[curr] = '\0';
		bool ok;
		yasl_int tmp = parseint64(buffer, &ok);
		YASL_ASSERT(ok, "parsing int from validated buffer should never fail.");
		free(buffer);
		return tmp;
	}

	size_t curr = 0;
	for (size_t i = 0; i < len; ++i) {
		if (chars[i] == '_') {
			continue;
		}
		buffer[curr++] = chars[i];
	}
	buffer[curr] = '\0';

	bool ok;
	yasl_int tmp = parseint64(buffer, &ok);
	YASL_ASSERT(ok, "parsing int from validated buffer should never fail.");
	free(buffer);
	return tmp;
}

#define UPPER(c) (0x61 <= (c) && (c) < 0x7B ? (c) & ~0x20 : (c))
#define LOWER(c) (0x41 <= curr && curr < 0x5B ? (c) | 0x20 : (c))

// TODO: this is very ASCII reliant. Clean these up.
#define DEFINE_STR_TO_X(name, fun) struct YASL_String *YASL_String_to##name(struct VM *vm, struct YASL_String *a) {\
	const size_t length = YASL_String_len(a);\
	const char *chars = YASL_String_chars(a);\
	size_t i = 0;\
	char curr;\
	char *ptr = (char *)malloc(length);\
\
	while (i < length) {\
		curr = chars[i];\
		ptr[i++] = fun(curr);\
	}\
\
	return YASL_String_new_take(vm, ptr, length);\
}

DEFINE_STR_TO_X(upper, UPPER);
DEFINE_STR_TO_X(lower, LOWER);

bool YASL_String_startswith(struct YASL_String *haystack, struct YASL_String *needle) {
	const size_t needle_len = YASL_String_len(needle);
	const char *haystack_chars = YASL_String_chars(haystack);
	const char *needle_chars = YASL_String_chars(needle);
	if ((YASL_String_len(haystack) < needle_len)) {
		return false;
	}
	size_t i = 0;
	while (i < needle_len) {
		if (haystack_chars[i] != needle_chars[i]) return false;
		i++;
	}

	return true;
}

bool YASL_String_endswith(struct YASL_String *haystack, struct YASL_String *needle) {
	const size_t haystack_len = YASL_String_len(haystack);
	const size_t needle_len = YASL_String_len(needle);
	if ((haystack_len < needle_len)) {
		return false;
	}
	size_t i = 0;
	while (i < needle_len) {
		if (CHARS(haystack)[i + haystack_len - needle_len]
		    != CHARS(needle)[i]) {
			return false;
		}
		i++;
	}

	return true;
}

#define STR_REPLACE(cond, post) \
	YASL_ASSERT(YASL_String_len(search_str) >= 1, "`search_str` must be non-empty");\
	unsigned char *str_ptr = (unsigned char *) CHARS(str);\
	const size_t str_len = YASL_String_len(str);\
	const char *search_str_ptr = CHARS(search_str);\
	const size_t search_len = YASL_String_len(search_str);\
	unsigned char *replace_str_ptr = (unsigned char *) CHARS(replace_str);\
	\
	YASL_ByteBuffer buff = NEW_BB(str_len);\
	size_t i = 0;\
	while (i < str_len) {\
		if (search_len <= str_len - i && memcmp(str_ptr + i, search_str_ptr, search_len) == 0 && (cond)) {\
			YASL_ByteBuffer_extend(&buff, replace_str_ptr, YASL_String_len(replace_str));\
			i += search_len;\
			++*replacements;\
			post;\
		} else {\
			YASL_ByteBuffer_add_byte(&buff, str_ptr[i++]);\
		}\
	}\
	return YASL_String_new_takebb(vm, &buff);

// Caller makes sure search_str is at least length 1.
struct YASL_String *YASL_String_replace_fast_default(struct VM *vm, struct YASL_String *str, struct YASL_String *search_str,
					     struct YASL_String *replace_str, int *replacements) {
	STR_REPLACE(true, {})
}

// Caller makes sure search_str is at least length 1.
struct YASL_String *YASL_String_replace_fast(struct VM *vm, struct YASL_String *str, struct YASL_String *search_str,
					     struct YASL_String *replace_str, int *replacements, yasl_int max) {
	STR_REPLACE(max > 0, max--)
}

yasl_int YASL_String_count(struct YASL_String *haystack, struct YASL_String *needle) {
	const size_t nLen = YASL_String_len(needle);
	const size_t hLen = YASL_String_len(haystack);
	const char *haystack_chars = YASL_String_chars(haystack);
	int64_t count = 0;
	for(int64_t i = 0; i + nLen <= hLen; i++) {
		if(!memcmp(CHARS(needle), haystack_chars + i, nLen)) {
			count++;
			i += nLen-1;
		}
	}

	return count;
}

#define DEF_STR_SPLIT_DEFAULT(COND, POST) \
	const size_t haystack_len = YASL_String_len(haystack);\
	const char *haystack_chars = YASL_String_chars(haystack);\
	size_t end = 0, start = 0;\
	while (COND) {\
		while (end < haystack_len && iswhitespace(haystack_chars[end])) {\
			end++;\
		}\
		if (end >= haystack_len) break;\
		start = end;\
		while (end < haystack_len && !iswhitespace(haystack_chars[end])) {\
			end++;\
		}\
		struct YASL_Object to = YASL_STR(\
			YASL_String_new_substring(vm, haystack, start, end));\
		YASL_List_push(data, to);\
		POST;\
	}

void YASL_String_split_default(struct VM *vm, struct YASL_List *data, struct YASL_String *haystack) {
	DEF_STR_SPLIT_DEFAULT(true, {});
}

void YASL_String_split_default_max(struct VM *vm, struct YASL_List *data, struct YASL_String *haystack, yasl_int max_splits) {
	YASL_ASSERT(max_splits >= 0, "max_splits should be greater than or equal to 0");
	DEF_STR_SPLIT_DEFAULT(max_splits > 0, max_splits--);
	start = end;
	end = haystack_len;
	while (start < haystack_len && iswhitespace(haystack_chars[start])) {
		start++;
	}
	if (start >= end) return;
	struct YASL_Object to = YASL_STR(\
			YASL_String_new_substring(vm, haystack, start, end));
	YASL_List_push(data, to);
}

#define DEF_STR_SPLIT(COND, POST) \
	YASL_ASSERT(YASL_String_len(needle) != 0, "needle must have non-zero length");\
	const size_t needle_len = YASL_String_len(needle);\
	const char *haystack_chars = YASL_String_chars(haystack);\
	const char *needle_chars = YASL_String_chars(needle);\
	int64_t end = 0, start = 0;\
	while (end + needle_len <= YASL_String_len(haystack) && (COND)) {\
		if (!memcmp(haystack_chars + end, needle_chars, needle_len)) {\
			struct YASL_Object to = YASL_STR(\
				YASL_String_new_substring(vm, haystack, start, end));\
			YASL_List_push(data, to);\
			end += needle_len;\
			start = end;\
			POST;\
		} else {\
			end++;\
		}\
	}\
	end = YASL_String_len(haystack);\
	struct YASL_Object to = YASL_STR(\
		YASL_String_new_substring(vm, haystack, start, end));\
	YASL_List_push(data, to);



// Caller makes sure needle is not 0 length
void YASL_String_split_fast(struct VM *vm, struct YASL_List *data, struct YASL_String *haystack, struct YASL_String *needle) {
	DEF_STR_SPLIT(true, {});
}

// Caller makes sure needle is not 0 length
void YASL_String_split_max_fast(struct VM *vm, struct YASL_List *data, struct YASL_String *haystack, struct YASL_String *needle, yasl_int max_splits) {
	YASL_ASSERT(max_splits >= 0, "max_splits should be greater than or equal to 0");
	DEF_STR_SPLIT(max_splits > 0, max_splits--);
}

struct YASL_String *YASL_String_ltrim_default(struct VM *vm, struct YASL_String *haystack) {
	const size_t haystack_len = YASL_String_len(haystack);
	const char *haystack_chars = YASL_String_chars(haystack);
	size_t start = 0;
	while (haystack_len - start >= 1 && iswhitespace(haystack_chars[start])) {
		start++;
	}

	return YASL_String_new_substring(vm, haystack, start, haystack_len);
}

struct YASL_String *YASL_String_ltrim(struct VM *vm, struct YASL_String *haystack, struct YASL_String *needle) {
	const size_t haystack_len = YASL_String_len(haystack);
	const size_t needle_len = YASL_String_len(needle);
	const char *haystack_chars = YASL_String_chars(haystack);
	const char *needle_chars = YASL_String_chars(needle);
	size_t start = 0;
	while(haystack_len - start >= needle_len &&
	      !memcmp(haystack_chars + start, needle_chars, needle_len)) {
		start += needle_len;
	}

	return YASL_String_new_substring(vm, haystack, start, haystack_len);
}

struct YASL_String *YASL_String_rtrim_default(struct VM *vm, struct YASL_String *haystack) {
	const char *haystack_chars = YASL_String_chars(haystack);
	size_t end = YASL_String_len(haystack);
	while (end >= 1 && iswhitespace(haystack_chars[end - 1])) {
		end--;
	}

	return YASL_String_new_substring(vm, haystack, 0, end);
}

struct YASL_String *YASL_String_rtrim(struct VM *vm, struct YASL_String *haystack, struct YASL_String *needle) {
	const size_t needle_len = YASL_String_len(needle);
	const char *haystack_chars = YASL_String_chars(haystack);
	const char *needle_chars = YASL_String_chars(needle);
	size_t end = YASL_String_len(haystack);
	while (end >= needle_len &&
	       !memcmp(haystack_chars + end - needle_len, needle_chars, needle_len)) {
		end -= needle_len;
	}

	return YASL_String_new_substring(vm, haystack, 0, end);
}

struct YASL_String *YASL_String_trim_default(struct VM *vm, struct YASL_String *haystack) {
	const size_t haystack_len = YASL_String_len(haystack);
	const char *haystack_chars = YASL_String_chars(haystack);
	size_t start = 0;
	while (haystack_len - start >= 1 && iswhitespace(haystack_chars[start])) {
		start++;
	}

	size_t end = haystack_len;
	while (end >= 1 && iswhitespace(haystack_chars[end - 1])) {
		end--;
	}

	return YASL_String_new_substring(vm, haystack, start, end);
}

struct YASL_String *YASL_String_trim(struct VM *vm, struct YASL_String *haystack, struct YASL_String *needle) {
	const size_t haystack_len = YASL_String_len(haystack);
	const size_t needle_len = YASL_String_len(needle);
	const char *haystack_chars = YASL_String_chars(haystack);
	const char *needle_chars = YASL_String_chars(needle);
	size_t start = 0;
	while (haystack_len - start >= needle_len &&
	       !memcmp(haystack_chars + start, needle_chars, needle_len)) {
		start += needle_len;
	}

	size_t end = haystack_len;
	while (end >= needle_len &&
	       !memcmp(haystack_chars + end - needle_len, needle_chars, needle_len)) {
		end -= needle_len;
	}

	return YASL_String_new_substring(vm, haystack, start, end);
}

// Caller ensures num is greater than or equal to zero
struct YASL_String *YASL_String_rep_fast(struct VM *vm, struct YASL_String *string, yasl_int num) {
	YASL_ASSERT(num >= 0, "num must be non-negative");
	const size_t string_len = YASL_String_len(string);
	size_t size = num * string_len;
	char *str = (char *)malloc(size);
	for (size_t i = 0; i < size; i += string_len) {
		memcpy(str + i, CHARS(string), string_len);
	}

	return YASL_String_new_take(vm, str, size);
}

