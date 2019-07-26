#include "YASL_string.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "YASL_list.h"
#include "interpreter/YASL_Object.h"
#include "data-structures/YASL_bytebuffer.h"

#define iswhitespace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\v' || (c) == '\r')
#define isalphabetic
#define isnumeric

size_t yasl_string_len(const struct YASL_String *const str) {
	return (size_t)(str->end - str->start);
}

int64_t yasl_string_cmp(const struct YASL_String *const left, const struct YASL_String *const right) {
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

struct YASL_String *str_new_substring(const size_t start, const size_t end, const struct YASL_String *const string) {
	struct YASL_String *str = (struct YASL_String *) malloc(sizeof(struct YASL_String));
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

struct YASL_String *str_new_sized(const size_t base_size, const char *const ptr) {
	struct YASL_String *str = (struct YASL_String *) malloc(sizeof(struct YASL_String));
	str->start = 0;
	str->end = base_size;
	str->str = (char *) ptr;
	str->on_heap = 0;
	str->rc = rc_new();
	return str;
}

struct YASL_String* str_new_sized_heap(const size_t start, const size_t end, const char *const mem) {
	struct YASL_String *str = (struct YASL_String *) malloc(sizeof(struct YASL_String));
	str->start = start;
	str->end = end;
	str->str = (char *) mem;
	str->on_heap = 1;
	str->rc = rc_new();
	return str;
}

void str_del_data(struct YASL_String *const str) {
	if (str->on_heap) free((void *) str->str);
}

void str_del_rc(struct YASL_String *const str) {
	rc_del(str->rc);
	free(str);
}

void str_del(struct YASL_String *const str) {
	if (str->on_heap) free((void *) str->str);
	rc_del(str->rc);
	free(str);
}


/********************************************************************************
 *                                                                              *
 *                             String Functions                                 *
 *                                                                              *
 ********************************************************************************/


bool isvaliddouble(const char *str) {
	long len = strlen(str);
	bool hasdot = false;
	bool hase = false;
	for (size_t i = 0; i < strlen(str); i++) {
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
	return hasdot && isdigit(str[len-1]) && isdigit(str[0]);
}

int64_t str_find_index(const struct YASL_String *const haystack, const struct YASL_String *const needle) {
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

static yasl_float parsedouble(const char *str, int *error) {
	*error = 1;
	if (!strcmp(str, "inf") || !strcmp(str, "+inf")) return INFINITY;
	else if (!strcmp(str, "-inf")) return -INFINITY;
	else if (str[0] == '-' && isvaliddouble(str+1))
		return -strtod(str+1, NULL);
	else if (str[0] == '+' && isvaliddouble(str+1))
		return +strtod(str+1, NULL);
	else if (isvaliddouble(str))
		return strtod(str, NULL);
	*error = 0;
	return NAN;
}

static yasl_int parseint64(const char *str, int *error) {
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
	*error = str + len == end;
	return str + len == end ? result : 0;
}

yasl_float string_tofloat(struct YASL_String *str) {
	char *buffer = (char *)malloc(yasl_string_len(str) + 1);
	if (!isdigit(str->str[str->start])) {
		free(buffer);
		return NAN;
	}
	size_t curr = 0;
	for (size_t i = 0; i < yasl_string_len(str); ++i) {
		if (str->str[str->start + i] == '_' && str->str[str->start + i - 1] != '.') {
			continue;
		}
		buffer[curr++] = str->str[str->start + i];
	}
	buffer[curr] = '\0';
	int ok = 1;
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

yasl_int string_toint(struct YASL_String *str) {
	char *buffer = (char *)malloc(yasl_string_len(str) + 1);

	if (yasl_string_len(str) <= 2) {
		memcpy(buffer, str->str + str->start, yasl_string_len(str));
		buffer[yasl_string_len(str)] = '\0';
		int ok;
		yasl_int tmp = parseint64(buffer, &ok);
		free(buffer);
		return tmp;
	}

	if (str->str[str->start + 0] == '0' && (isalpha(str->str[str->start + 1]))) {
		size_t curr = 2;
		buffer[0] = str->str[str->start + 0];
		buffer[1] = str->str[str->start + 1];
		for (size_t i = 2; i < yasl_string_len(str); ++i) {
			if (str->str[str->start + i] == '_') {
				continue;
			}
			buffer[curr++] = str->str[str->start + i];
		}
		buffer[curr] = '\0';
		int ok;
		yasl_int tmp = parseint64(buffer, &ok);
		free(buffer);
		return tmp;
	}

	size_t curr = 0;
	for (size_t i = 0; i < yasl_string_len(str); ++i) {
		if (str->str[str->start + i] == '_') {
			continue;
		}
		buffer[curr++] = str->str[str->start + i];
	}
	buffer[curr] = '\0';
	int ok;

	yasl_int tmp = parseint64(buffer, &ok);
	free(buffer);
	return tmp;
}

struct YASL_String *string_toupper(struct YASL_String *a) {
	size_t length = yasl_string_len(a);
	size_t i = 0;
	char curr;
	char *ptr = (char *)malloc(length);

	while (i < length) {
		curr = a->str[i + a->start];
		if (0x61 <= curr && curr < 0x7B) {
			ptr[i++] = curr & ~0x20;
		} else {
			ptr[i++] = curr;
		}
	}

	return str_new_sized_heap(0, length, ptr);
}

struct YASL_String *string_tolower(struct YASL_String *a) {
	size_t length = yasl_string_len(a);
	size_t i = 0;
	char curr;
	char *ptr = (char *)malloc(length);

	while (i < length) {
		curr = a->str[i + a->start];
		if (0x41 <= curr && curr < 0x5B) {
			ptr[i++] = curr | 0x20;
		} else {
			ptr[i++] = curr;
		}
	}
	return str_new_sized_heap(0, length, ptr);
}

bool string_isalnum(struct YASL_String *a) {
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	char curr;
	while (i < length) {
		curr = (a)->str[i++ + a->start];
		if (!isalpha(curr) && !isdigit(curr)) {
			return false;
		}
	}
	return true;
}

bool string_isal(struct YASL_String *a) {
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	char curr;
	while (i < length) {
		curr = ((a)->str[i++ + a->start]);
		if (!isalpha(curr)) {
			return false;
		}
	}
	return true;
}

bool string_isnum(struct YASL_String *a) {
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	char curr;
	while (i < length) {
		curr = (a)->str[i++ + a->start];
		if (!isdigit(curr)) {
			return false;
		}
	}
	return true;
}

bool string_isspace(struct YASL_String *a) {
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	unsigned char curr;
	while (i < length) {
		curr = (unsigned char) ((a)->str[i++ + a->start]);
		if (!iswhitespace(curr)) {
			return false;
		}
	}
	return true;
}

bool string_startswith(struct YASL_String *haystack, struct YASL_String *needle) {
	if ((yasl_string_len(haystack) < yasl_string_len(needle))) {
		return false;
	}
	size_t i = 0;
	while (i < yasl_string_len(needle)) {
		if (haystack->str[i + haystack->start] != needle->str[i + needle->start]) {
			return false;
		}
		i++;
	}

	return true;
}

bool string_endswith(struct YASL_String *haystack, struct YASL_String *needle) {
	if ((yasl_string_len(haystack) < yasl_string_len(needle))) {
		return false;
	}
	size_t i = 0;
	while (i < yasl_string_len(needle)) {
		if ((haystack)->str[i + haystack->start + yasl_string_len(haystack) - yasl_string_len(needle)]
		    != (needle)->str[i + needle->start]) {
			return false;
		}
		i++;
	}

	return true;
}

// Caller makes sure search_str is at least length 1.
struct YASL_String *string_replace(struct YASL_String *str, struct YASL_String *search_str, struct YASL_String *replace_str) {
	unsigned char *str_ptr = (unsigned char *) str->str + str->start;
	size_t str_len = yasl_string_len(str);
	const char *search_str_ptr = search_str->str + search_str->start;
	size_t search_len = yasl_string_len(search_str);
	unsigned char *replace_str_ptr = (unsigned char *) replace_str->str + replace_str->start;

	struct YASL_ByteBuffer *buff = bb_new(yasl_string_len(str));
	size_t i = 0;
	while (i < str_len) {
		if (search_len <= str_len - i && memcmp(str_ptr + i, search_str_ptr, search_len) == 0) {
			bb_append(buff, replace_str_ptr, yasl_string_len(replace_str));
			i += search_len;
		} else {
			bb_add_byte(buff, str_ptr[i++]);
		}
	}

	char *bytes = (char *)buff->bytes;
	buff->bytes = NULL;
	size_t count = buff->count;

	bb_del(buff);
	return str_new_sized_heap(0, count, bytes);
}

yasl_int string_count(struct YASL_String *haystack, struct YASL_String *needle) {
	size_t nLen = yasl_string_len(needle);
	size_t hLen = yasl_string_len(haystack);
	int64_t count = 0;
	for(int64_t i = 0; i + nLen <= hLen; i++) {
		if(memcmp(needle->str + needle->start, haystack->str + haystack->start + i, nLen) == 0) {
			count++;
			i += nLen-1;
		}
	}

	return count;
}

struct RC_UserData *string_split_default(struct YASL_String *haystack) {
	size_t end = 0, start = 0;
	struct RC_UserData *result = ls_new();
	while (true) {
		// printf("end: %d\n", (int)end);
		while (iswhitespace(*(haystack->str + haystack->start + end)) && end < yasl_string_len(haystack)) {
			end++;
		}
		if (end >= yasl_string_len(haystack)) break;
		start = end;
		while (!iswhitespace(*(haystack->str + haystack->start + end)) && end < yasl_string_len(haystack)) {
			end++;
		}
		struct YASL_Object to = YASL_STR(str_new_substring(start + haystack->start, end + haystack->start, haystack));
		ls_append((struct YASL_List *)result->data, to);
	}

	return result;
}

// Caller makes sure needle is not 0 length
struct RC_UserData *string_split(struct YASL_String *haystack, struct YASL_String *needle) {
	int64_t end = 0, start = 0;
	struct RC_UserData *result = ls_new();
	while (end + yasl_string_len(needle) <= yasl_string_len(haystack)) {
		if (!memcmp(haystack->str + haystack->start + end,
			    needle->str + needle->start,
			    yasl_string_len(needle))) {
			struct YASL_Object to = YASL_STR(str_new_substring(start + haystack->start, end + haystack->start, haystack));
			ls_append((struct YASL_List *)result->data, to);
			end += yasl_string_len(needle);
			start = end;
		} else {
			end++;
		}
	}
	struct YASL_Object to = YASL_STR(str_new_substring(start + haystack->start, end + haystack->start, haystack));
	ls_append((struct YASL_List *)result->data, to);

	return result;
}

struct YASL_String *string_ltrim_default(struct YASL_String *haystack) {
	int64_t start = 0;
	while (yasl_string_len(haystack) - start >= 1 && iswhitespace(*(haystack->str + haystack->start + start))) {
		start++;
	}

	int64_t end = yasl_string_len(haystack);

	return str_new_substring(haystack->start + start, haystack->start + end, haystack);
}

struct YASL_String *string_ltrim(struct YASL_String *haystack, struct YASL_String *needle) {
	int64_t start=0;
	while(yasl_string_len(haystack) - start >= yasl_string_len(needle) &&
	      !memcmp(haystack->str + haystack->start + start,
		      needle->str + needle->start,
		      yasl_string_len(needle))) {
		start += yasl_string_len(needle);
	}

	return str_new_substring(haystack->start + start, haystack->start + yasl_string_len(haystack),
				 haystack);
}

struct YASL_String *string_rtrim_default(struct YASL_String *haystack) {
	int64_t start = 0;

	int64_t end = yasl_string_len(haystack);
	while (end >= 1 && iswhitespace(*(haystack->str + haystack->start + end - 1))) {
		end--;
	}

	return str_new_substring(haystack->start + start, haystack->start + end, haystack);
}

struct YASL_String *string_rtrim(struct YASL_String *haystack, struct YASL_String *needle) {
	size_t end = yasl_string_len(haystack);
	while (end >= yasl_string_len(needle) &&
	       !memcmp(haystack->str + haystack->start + end - yasl_string_len(needle),
		       needle->str + needle->start,
		       yasl_string_len(needle))) {
		end -= yasl_string_len(needle);
	}

	return str_new_substring(haystack->start, haystack->start + end, haystack);
}

struct YASL_String *string_trim_default(struct YASL_String *haystack) {
	int64_t start = 0;
	while (yasl_string_len(haystack) - start >= 1 && iswhitespace(*(haystack->str + haystack->start + start))) {
		start++;
	}

	int64_t end = yasl_string_len(haystack);
	while (end >= 1 && iswhitespace(*(haystack->str + haystack->start + end - 1))) {
		end--;
	}

	return str_new_substring(haystack->start + start, haystack->start + end, haystack);
}

struct YASL_String *string_trim(struct YASL_String *haystack, struct YASL_String *needle) {
	int64_t start = 0;
	while (yasl_string_len(haystack) - start >= yasl_string_len(needle) &&
	       !memcmp(haystack->str + haystack->start + start,
		       needle->str + needle->start,
		       yasl_string_len(needle))) {
		start += yasl_string_len(needle);
	}

	size_t end = yasl_string_len(haystack);
	while (end >= yasl_string_len(needle) &&
	       !memcmp(haystack->str + haystack->start + end - yasl_string_len(needle),
		       needle->str + needle->start,
		       yasl_string_len(needle))) {
		end -= yasl_string_len(needle);
	}

	return str_new_substring(haystack->start + start, haystack->start + end, haystack);
}

// Caller ensures num is greater than or equal to zero
struct YASL_String *string_rep(struct YASL_String *string, yasl_int num) {
	size_t size = num * yasl_string_len(string);
	char *str = (char *)malloc(size);
	for (size_t i = 0; i < size; i += yasl_string_len(string)) {
		memcpy(str + i, string->str + string->start, yasl_string_len(string));
	}

	return str_new_sized_heap(0, size, str);
}

