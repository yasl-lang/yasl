#include "str_methods.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "bytebuffer/bytebuffer.h"
#include "YASL_string.h"
#include "yasl_state.h"
#include "interpreter/VM.h"
#include "interpreter/YASL_Object.h"
#include "interpreter/list.h"
#include "YASL_string.h"

#define iswhitespace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\v' || (c) == '\r')

int str___get(struct YASL_State *S) {
	struct YASL_Object index = vm_pop((struct VM *)S);
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.__get");
	String_t *str = YASL_GETSTR(vm_pop((struct VM *)S));
	if (index.type != Y_INT) {
		return -1;
		VM_PUSH((struct VM *)S, YASL_UNDEF());
	} else if (YASL_GETINT(index) < -(yasl_int)yasl_string_len(str) || YASL_GETINT(index) >= (yasl_int)yasl_string_len(str)) {
		printf("IndexError\n");
		return -1;
		VM_PUSH((struct VM *)S, YASL_UNDEF());
	} else {
		if (YASL_GETINT(index) >= 0)
			VM_PUSH((struct VM *)S, YASL_STR(
				str_new_substring(str->start + YASL_GETINT(index), str->start + YASL_GETINT(index) + 1,
						  str)));
		else
			VM_PUSH((struct VM *)S,
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
	} else if (YASL_GETINT(start_index) < -(yasl_int)yasl_string_len(str) ||
		   YASL_GETINT(start_index) > (yasl_int)yasl_string_len(str)) {
		return -1;
	} else if (YASL_GETINT(end_index) < -(yasl_int)yasl_string_len(str) || YASL_GETINT(end_index) > (yasl_int)yasl_string_len(str)) {
		return -1;
	}

	int64_t start = YASL_GETINT(start_index) < 0 ? YASL_GETINT(start_index) + (yasl_int)yasl_string_len(str) : YASL_GETINT(
		start_index);
	int64_t end =
		YASL_GETINT(end_index) < 0 ? YASL_GETINT(end_index) + (yasl_int)yasl_string_len(str) : YASL_GETINT(end_index);

	if (start > end) {
		return -1;
	}

	// TODO: fix bug with possible mem leak here
	VM_PUSH((struct VM *)S, YASL_STR(str_new_substring(str->start + start, str->start + end, str)));

	return 0;
}

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

yasl_float parsedouble(const char *str, int *error) {
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

int64_t parseint64(const char *str, int *error) {
	int64_t result;
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

int str_tofloat(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tofloat");
	String_t *str = YASL_GETSTR(vm_pop((struct VM *)S));
	char *buffer = (char *)malloc(yasl_string_len(str) + 1);
	if (!isdigit(str->str[str->start])) {
		free(buffer);
		VM_PUSH((struct VM *)S, YASL_FLOAT(NAN));
		return 0;
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
		VM_PUSH((struct VM *)S, YASL_FLOAT(result));
		free(buffer);
		return 0;
	}

	result = parseint64(buffer, &ok);
	if (ok) {
		VM_PUSH((struct VM *)S, YASL_FLOAT(result));
		free(buffer);
		return 0;
	}

	VM_PUSH((struct VM *)S, YASL_FLOAT(NAN));
	free(buffer);
	return 0;
}


int str_toint(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.toint");
	String_t *str = vm_popstr((struct VM *)S);
	char *buffer = (char *)malloc(yasl_string_len(str) + 1);

	if (yasl_string_len(str) <= 2) {
		memcpy(buffer, str->str + str->start, yasl_string_len(str));
		buffer[yasl_string_len(str)] = '\0';
		int ok;
		VM_PUSH((struct VM *)S, YASL_INT(parseint64(buffer, &ok)));
		free(buffer);
		return 0;
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
		VM_PUSH((struct VM *)S, YASL_INT(parseint64(buffer, &ok)));
		free(buffer);
		return 0;
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
	VM_PUSH((struct VM *)S, YASL_INT(parseint64(buffer, &ok)));
	free(buffer);
	return 0;
}

int str_tobool(struct YASL_State* S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tobool");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	if (yasl_string_len(a) == 0) {
		VM_PUSH((struct VM *)S, YASL_BOOL(0));
	} else {
		VM_PUSH((struct VM *)S, YASL_BOOL(1));
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
	char *ptr = (char *)malloc(length);

	while (i < length) {
		curr = a->str[i + a->start];
		if (0x61 <= curr && curr < 0x7B) {
			ptr[i++] = curr & ~0x20;
		} else {
			ptr[i++] = curr;
		}
	}

	VM_PUSH((struct VM *)S, YASL_STR(str_new_sized_heap(0, length, ptr)));
	return 0;
}

int str_tolower(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tolower");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	int64_t length = yasl_string_len(a);
	int64_t i = 0;
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
	VM_PUSH((struct VM *)S, YASL_STR(str_new_sized_heap(0, length, ptr)));
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
			VM_PUSH((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
	}
	VM_PUSH((struct VM *)S, YASL_BOOL(1));
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
			VM_PUSH((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
	}
	VM_PUSH((struct VM *)S, YASL_BOOL(1));
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
			VM_PUSH((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
	}
	VM_PUSH((struct VM *)S, YASL_BOOL(1));
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
			VM_PUSH((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
	}
	VM_PUSH((struct VM *)S, YASL_BOOL(1));
	return 0;
}

int str_startswith(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.startswith");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.startswith");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	if ((yasl_string_len(haystack) < yasl_string_len(needle))) {
		VM_PUSH((struct VM *)S, YASL_BOOL(0));
		return 0;
	}
	size_t i = 0;
	while (i < yasl_string_len(needle)) {
		if (haystack->str[i + haystack->start] != needle->str[i + needle->start]) {
			VM_PUSH((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
		i++;
	}
	VM_PUSH((struct VM *)S, YASL_BOOL(1));
	return 0;
}

int str_endswith(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.endswith");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.endswith");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	if ((yasl_string_len(haystack) < yasl_string_len(needle))) {
		VM_PUSH((struct VM *)S, YASL_BOOL(0));
		return 0;
	}
	size_t i = 0;
	while (i < yasl_string_len(needle)) {
		if ((haystack)->str[i + haystack->start + yasl_string_len(haystack) - yasl_string_len(needle)]
		    != (needle)->str[i + needle->start]) {
			VM_PUSH((struct VM *)S, YASL_BOOL(0));
			return 0;
		}
		i++;
	}
	VM_PUSH((struct VM *)S, YASL_BOOL(1));
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
	const char *search_str_ptr = search_str->str + search_str->start;
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

	char *new_str = (char *)malloc(buff->count);
	memcpy(new_str, buff->bytes, buff->count);
	VM_PUSH((struct VM *)S, YASL_STR(str_new_sized_heap(0, buff->count, new_str)));

	bb_del(buff);
	return 0;
}

int str_search(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.search");
	String_t *needle = vm_popstr((struct VM *)S);
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.search");
	String_t *haystack = vm_popstr((struct VM *)S);

	int64_t index = str_find_index(haystack, needle);
	if (index != -1) VM_PUSH((struct VM *)S, YASL_INT(index));
	else VM_PUSH((struct VM *)S, YASL_UNDEF());
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
		  struct YASL_Object to = YASL_STR(str_new_substring(start + haystack->start, end + haystack->start, haystack));
		  ls_append((struct List *)result->data, to);
			end += yasl_string_len(needle);
			start = end;
		} else {
			end++;
		}
	}
	struct YASL_Object to = YASL_STR(str_new_substring(start + haystack->start, end + haystack->start, haystack));
	ls_append((struct List *)result->data, to);
	VM_PUSH((struct VM *)S, YASL_LIST(result));
	return 0;
}

static int str_ltrim_default(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.trim");
	struct YASL_Object top = vm_peek((struct VM *)S);
	inc_ref(&top);
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	int64_t start = 0;
	while (yasl_string_len(haystack) - start >= 1 && iswhitespace(*(haystack->str + haystack->start + start))) {
		start++;
	}

	int64_t end = yasl_string_len(haystack);

	VM_PUSH((struct VM *)S, YASL_STR(str_new_substring(haystack->start + start, haystack->start + end, haystack)));
	dec_ref(&top);
	return 0;
}

int str_ltrim(struct YASL_State *S) {
	if (YASL_ISUNDEF(vm_peek((struct VM *)S))) {
		vm_pop((struct VM *)S);
		return str_ltrim_default(S);
	}
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

        VM_PUSH((struct VM *)S,
                YASL_STR(str_new_substring(haystack->start + start, haystack->start + yasl_string_len(haystack),
                        haystack)));

    return 0;
}

static int str_rtrim_default(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.trim");
	struct YASL_Object top = vm_peek((struct VM *)S);
	inc_ref(&top);
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	int64_t start = 0;

	int64_t end = yasl_string_len(haystack);
	while (end >= 1 && iswhitespace(*(haystack->str + haystack->start + end - 1))) {
		end--;
	}

	VM_PUSH((struct VM *)S, YASL_STR(str_new_substring(haystack->start + start, haystack->start + end, haystack)));
	dec_ref(&top);
	return 0;
}

int str_rtrim(struct YASL_State *S) {
	if (YASL_ISUNDEF(vm_peek((struct VM *)S))) {
		vm_pop((struct VM *)S);
		return str_rtrim_default(S);
	}
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.rtrim");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.rtrim");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	size_t end = yasl_string_len(haystack);
	while (end >= yasl_string_len(needle) &&
	       !memcmp(haystack->str + haystack->start + end - yasl_string_len(needle),
		       needle->str + needle->start,
		       yasl_string_len(needle))) {
		end -= yasl_string_len(needle);
	}

	VM_PUSH((struct VM *)S, YASL_STR(str_new_substring(haystack->start, haystack->start + end, haystack)));
	return 0;
}



static int str_trim_default(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.trim");
	struct YASL_Object top = vm_peek((struct VM *)S);
	inc_ref(&top);
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	int64_t start = 0;
	while (yasl_string_len(haystack) - start >= 1 && iswhitespace(*(haystack->str + haystack->start + start))) {
		start++;
	}

	int64_t end = yasl_string_len(haystack);
	while (end >= 1 && iswhitespace(*(haystack->str + haystack->start + end - 1))) {
		end--;
	}

	VM_PUSH((struct VM *)S, YASL_STR(str_new_substring(haystack->start + start, haystack->start + end, haystack)));
	dec_ref(&top);
	return 0;
}

int str_trim(struct YASL_State *S) {
	if (YASL_ISUNDEF(vm_peek((struct VM *)S))) {
		vm_pop((struct VM *)S);
		return str_trim_default(S);
	}
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.trim");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.trim");
	struct YASL_Object top = vm_peek((struct VM *)S);
	inc_ref(&top);
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

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

	// TODO: fix possible mem leak here
	VM_PUSH((struct VM *)S, YASL_STR(str_new_substring(haystack->start + start, haystack->start + end, haystack)));
	dec_ref(&top);
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
	char *str = (char *)malloc(size);
	for (size_t i = 0; i < size; i += yasl_string_len(string)) {
		memcpy(str + i, string->str + string->start, yasl_string_len(string));
	}
	VM_PUSH((struct VM *)S, YASL_STR(str_new_sized_heap(0, size, str)));
	return 0;
}
