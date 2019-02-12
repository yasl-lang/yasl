#include "str_methods.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bytebuffer/bytebuffer.h"
#include "interpreter/YASL_string/YASL_string.h"
#include "yasl_state.h"
#include "interpreter/VM/VM.h"
#include "interpreter/YASL_Object/YASL_Object.h"
#include "interpreter/list/list.h"

int str___get(struct YASL_State *S) {
	struct YASL_Object index = vm_pop((struct VM *)S);
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.__get");
	String_t *str = YASL_GETSTR(vm_pop((struct VM *)S));
	if (index.type != Y_INT) {
		return -1;
		vm_push((struct VM *)S, YASL_UNDEF());
	} else if (YASL_GETINT(index) < -yasl_string_len(str) || YASL_GETINT(index) >= yasl_string_len(str)) {
		return -1;
		vm_push((struct VM *)S, YASL_UNDEF());
	} else {
		if (YASL_GETINT(index) >= 0)
			vm_push((struct VM *)S, YASL_STR(
				str_new_substring(str->start + YASL_GETINT(index), str->start + YASL_GETINT(index) + 1,
						  str)));
		else
			vm_push((struct VM *)S,
				YASL_STR(str_new_substring(str->start + YASL_GETINT(index) + yasl_string_len(str),
							   str->start + YASL_GETINT(index) + yasl_string_len(str) + 1,
							   str)));
	}
	return 0;
}

int str_slice(struct YASL_State *S) {
	struct YASL_Object end_index = vm_pop((struct VM *)S);
	struct YASL_Object start_index = vm_pop((struct VM *)S);
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.slice");
	String_t *str = YASL_GETSTR(vm_pop((struct VM *)S));
	if (!YASL_ISINT(start_index) || !YASL_ISINT(end_index)) {
		return -1;
	} else if (YASL_GETINT(start_index) < -yasl_string_len(str) ||
		   YASL_GETINT(start_index) > yasl_string_len(str)) {
		return -1;
	} else if (YASL_GETINT(end_index) < -yasl_string_len(str) || YASL_GETINT(end_index) > yasl_string_len(str)) {
		return -1;
	}

	int64_t start = YASL_GETINT(start_index) < 0 ? YASL_GETINT(start_index) + yasl_string_len(str) : YASL_GETINT(
		start_index);
	int64_t end =
		YASL_GETINT(end_index) < 0 ? YASL_GETINT(end_index) + yasl_string_len(str) : YASL_GETINT(end_index);

	if (start > end) {
		return -1;
	}

	// TODO: fix bug with possible mem leak here
	vm_push((struct VM *)S, YASL_STR(str_new_substring(str->start + start, str->start + end, str)));

	return 0;
}

int isvaliddouble(const char *str) {
	long len = strlen(str);
	int hasdot = 0;
	for (size_t i = 0; i < strlen(str); i++) {
		if ((!isdigit(str[i]) && str[i] != '.') || (hasdot && str[i] == '.')) {
			return 0;
		}
		if (str[i] == '.') hasdot = 1;
	}
	return hasdot && isdigit(str[len-1]) && isdigit(str[0]);
}

double parsedouble(const char *str) {
	if (!strcmp(str, "inf") || !strcmp(str, "+inf")) return INFINITY;
	else if (!strcmp(str, "-inf")) return -INFINITY;
	else if (str[0] == '-' && isvaliddouble(str+1))
		return -strtod(str+1, NULL);
	else if (str[0] == '+' && isvaliddouble(str+1))
		return +strtod(str+1, NULL);
	else if (isvaliddouble(str))	return strtod(str, NULL);
	return NAN;
}

int64_t parseint64(const char *str) {
	int64_t result;
	size_t len = strlen(str);
	char *end;
	if (len > 2 && str[0] == '0' && str[1] == 'x') {
		result = strtoll(str + 2, &end, 16);
	} else if (len > 2 && str[0] == '0' && str[1] == 'b') {
		result = strtoll(str + 2, &end, 2);
	} else {
		result = strtoll(str, &end, 10);
	}
	// printf("%d, %d", str + len, end);
	return str + len == end ? result : 0;
}

int str_tofloat(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tofloat");
	String_t *str = YASL_GETSTR(vm_pop((struct VM *)S));
	char *buffer = malloc(yasl_string_len(str) + 1);
	memcpy(buffer, str->str + str->start, yasl_string_len(str));
	buffer[yasl_string_len(str)] = '\0';
	vm_push((struct VM *)S, YASL_FLOAT(parsedouble(buffer)));
	free(buffer);
	return 0;
}


int str_toint(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.toint");
	String_t *str = vm_popstr((struct VM *)S);
	char *buffer = malloc(yasl_string_len(str) + 1);
	memcpy(buffer, str->str + str->start, yasl_string_len(str));
	buffer[yasl_string_len(str)] = '\0';
	vm_push((struct VM *)S, YASL_INT(parseint64(buffer)));
	free(buffer);
	return 0;
}

int str_tobool(struct YASL_State* S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tobool");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	if (yasl_string_len(a) == 0) {
		vm_push((struct VM *)S, YASL_BOOL(0));
	} else {
		vm_push((struct VM *)S, YASL_BOOL(1));
	}
	return 0;
}

int str_tostr(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tostr");
	return 0;
}

int str_toupper(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.toupper");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	char curr;
	char *ptr = malloc(length);

	while (i < length) {
		curr = a->str[i + a->start];
		if (0x61 <= curr && curr < 0x7B) {
			ptr[i++] = curr & ~0x20;
		} else {
			ptr[i++] = curr;
		}
	}

	vm_push((struct VM *)S, YASL_STR(str_new_sized_heap(0, length, ptr)));
	return 0;
}

int str_tolower(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tolower");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	char curr;
	char *ptr = malloc(length);

	while (i < length) {
		curr = a->str[i + a->start];
		if (0x41 <= curr && curr < 0x5B) {
			ptr[i++] = curr | 0x20;
		} else {
			ptr[i++] = curr;
		}
	}
	vm_push((struct VM *)S, YASL_STR(str_new_sized_heap(0, length, ptr)));
	return 0;
}

int str_isalnum(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.isalnum");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	char curr;
	while (i < length) {
		curr = (a)->str[i++ + a->start];
		if (curr < 0x30 || (0x3A <= curr && curr < 0x41) || (0x5B <= curr && curr < 0x61) || (0x7B <= curr)) {
			vm_push((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
	}
	vm_push((struct VM *)S, YASL_BOOL(1));
	return 0;
}

int str_isal(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.isal");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	char curr;
	while (i < length) {
		curr = ((a)->str[i++ + a->start]);
		if (curr < 0x41 || (0x5B <= curr && curr < 0x61) || (0x7B <= curr)) {
			vm_push((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
	}
	vm_push((struct VM *)S, YASL_BOOL(1));
	return 0;
}

int str_isnum(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.isnum");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	char curr;
	while (i < length) {
		curr = (a)->str[i++ + a->start];
		if (curr < 0x30 || 0x3A <= curr) {
			vm_push((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
	}
	vm_push((struct VM *)S, YASL_BOOL(1));
	return 0;
}

int str_isspace(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.isspace");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
	unsigned char curr;
	while (i < length) {
		curr = (unsigned char) ((a)->str[i++ + a->start]);
		if (curr <= 0x08 || (0x0D < curr && curr != 0x20 && curr != 0x85 && curr != 0xA0)) {
			vm_push((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
	}
	vm_push((struct VM *)S, YASL_BOOL(1));
	return 0;
}

int str_startswith(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.startswith");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.startswith");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	if ((yasl_string_len(haystack) < yasl_string_len(needle))) {
		vm_push((struct VM *)S, YASL_BOOL(0));
		return 0;
	}
	int64_t i = 0;
	while (i < yasl_string_len(needle)) {
		if (haystack->str[i + haystack->start] != needle->str[i + needle->start]) {
			vm_push((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
		i++;
	}
	vm_push((struct VM *)S, YASL_BOOL(1));
	return 0;
}

int str_endswith(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.endswith");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.endswith");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	if ((yasl_string_len(haystack) < yasl_string_len(needle))) {
		vm_push((struct VM *)S, YASL_BOOL(0));
		return 0;
	}
	int64_t i = 0;
	while (i < yasl_string_len(needle)) {
		if ((haystack)->str[i + haystack->start + yasl_string_len(haystack) - yasl_string_len(needle)]
		    != (needle)->str[i + needle->start]) {
			vm_push((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
		i++;
	}
	vm_push((struct VM *)S, YASL_BOOL(1));
	return 0;
}

int str_replace(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.replace");
	String_t *replace_str = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.replace");
	String_t *search_str = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.replace");
	String_t *str = YASL_GETSTR(vm_pop((struct VM *)S));

	unsigned char *str_ptr = (unsigned char *) str->str + str->start;
	size_t str_len = (size_t) yasl_string_len(str);
	char *search_str_ptr = search_str->str + search_str->start;
	size_t search_len = (size_t) yasl_string_len(search_str);
	unsigned char *replace_str_ptr = (unsigned char *) replace_str->str + replace_str->start;
	if (search_len < 1) {
		printf("Error: str.replace(...) expected search string with length at least 1\n");
		return -1;
	}

	ByteBuffer *buff = bb_new(yasl_string_len(str));
	size_t i = 0;
	while (i < str_len) {
		if (search_len <= str_len - i && memcmp(str_ptr + i, search_str_ptr, search_len) == 0) {
			bb_append(buff, replace_str_ptr, yasl_string_len(replace_str));
			i += search_len;
		} else {
			bb_add_byte(buff, str_ptr[i++]);
		}
	}

	char *new_str = malloc(buff->count);
	memcpy(new_str, buff->bytes, buff->count);
	vm_push((struct VM *)S, YASL_STR(str_new_sized_heap(0, buff->count, new_str)));

	bb_del(buff);
	return 0;
}

int str_search(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.search");
	String_t *needle = vm_popstr((struct VM *)S);
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.search");
	String_t *haystack = vm_popstr((struct VM *)S);

	int64_t index = str_find_index(haystack, needle);
	if (index != -1) vm_push((struct VM *)S, YASL_INT(index));
	else vm_push((struct VM *)S, YASL_UNDEF());
	return 0;
}

int str_count(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.count");
	String_t *needle = vm_popstr((struct VM *)S);
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.count");
	String_t *haystack = vm_popstr((struct VM *)S);

	int64_t nLen = yasl_string_len(needle);
	int64_t hLen = yasl_string_len(haystack);
	int64_t count = 0;
	for(int64_t i = 0; i + nLen <= hLen; i++) {
		if(memcmp(needle->str + needle->start, haystack->str + haystack->start + i, nLen) == 0) {
			count++;
			i += nLen-1;
		}
	}
	vm_pushint((struct VM *)S, count);
	return 0;
}

// TODO: fix all of these

int str_split(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.split");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.split");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));
	if (yasl_string_len(needle) == 0) {
		printf("Error: str.split(...) requires type %x of length > 0 as second argument\n", Y_STR);
		return -1;
	}
	int64_t end = 0, start = 0;
	struct RC_UserData *result = ls_new();
	while (end + yasl_string_len(needle) <= yasl_string_len(haystack)) {
		if (!memcmp(haystack->str + haystack->start + end,
			    needle->str + needle->start,
			    yasl_string_len(needle))) {
			ls_append(result->data,
				  YASL_STR(
					  str_new_substring(start + haystack->start, end + haystack->start, haystack)));
			end += yasl_string_len(needle);
			start = end;
		} else {
			end++;
		}
	}
	ls_append(result->data, YASL_STR(str_new_substring(start + haystack->start, end + haystack->start, haystack)));
	vm_push((struct VM *)S, YASL_LIST(result));
	return 0;
}


int str_ltrim(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.ltrim");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.ltrim");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	int64_t start=0;
	while(yasl_string_len(haystack) - start >= yasl_string_len(needle) &&
	        !memcmp(haystack->str + haystack->start + start,
	                needle->str + needle->start,
	                yasl_string_len(needle))) {
	    start += yasl_string_len(needle);
        }

        vm_push((struct VM *)S,
                YASL_STR(str_new_substring(haystack->start + start, haystack->start + yasl_string_len(haystack),
                        haystack)));

    return 0;
}

int str_rtrim(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.rtrim");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.rtrim");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	int64_t end = yasl_string_len(haystack);
	while (end >= yasl_string_len(needle) &&
	       !memcmp(haystack->str + haystack->start + end - yasl_string_len(needle),
		       needle->str + needle->start,
		       yasl_string_len(needle))) {
		end -= yasl_string_len(needle);
	}

	vm_push((struct VM *)S, YASL_STR(str_new_substring(haystack->start, haystack->start + end, haystack)));
	return 0;
}

int str_trim(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.split");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.split");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	int64_t start = 0;
	while (yasl_string_len(haystack) - start >= yasl_string_len(needle) &&
	       !memcmp(haystack->str + haystack->start + start,
		       needle->str + needle->start,
		       yasl_string_len(needle))) {
		start += yasl_string_len(needle);
	}

	int64_t end = yasl_string_len(haystack);
	while (end >= yasl_string_len(needle) &&
	       !memcmp(haystack->str + haystack->start + end - yasl_string_len(needle),
		       needle->str + needle->start,
		       yasl_string_len(needle))) {
		end -= yasl_string_len(needle);
	}

	// TODO: fix possible mem leak here
	vm_push((struct VM *)S, YASL_STR(str_new_substring(haystack->start + start, haystack->start + end, haystack)));

	return 0;
}

int str_repeat(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_INT, "str.repeat");
	yasl_int num = YASL_GETINT(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.repeat");
	String_t *string = YASL_GETSTR(vm_pop((struct VM *)S));

	if (num < 0) {
		return -1;
	}

	size_t size = num * yasl_string_len(string);
	char *str = malloc(size);
	for (size_t i = 0; i < size; i += yasl_string_len(string)) {
		memcpy(str + i, string->str + string->start, yasl_string_len(string));
	}
	vm_push((struct VM *)S, YASL_STR(str_new_sized_heap(0, size, str)));
	return 0;
}
