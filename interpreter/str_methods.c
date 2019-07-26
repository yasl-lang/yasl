#include "str_methods.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <data-structures/YASL_bytebuffer.h>

#include "data-structures/YASL_bytebuffer.h"
#include "data-structures/YASL_string.h"
#include "yasl_state.h"
#include "interpreter/VM.h"
#include "interpreter/YASL_Object.h"
#include "data-structures/YASL_list.h"
#include "data-structures/YASL_string.h"

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

int str_tofloat(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tofloat");
	String_t *str = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_FLOAT(string_tofloat(str)));
	return 0;
}

int str_toint(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.toint");
	String_t *str = vm_popstr((struct VM *)S);

	VM_PUSH((struct VM *)S, YASL_INT(string_toint(str)));
	return 0;
}

int str_tobool(struct YASL_State* S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tobool");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));
	VM_PUSH((struct VM *)S, YASL_BOOL(yasl_string_len(a) != 0));
	return 0;
}

int str_tostr(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tostr");
	return 0;
}

int str_toupper(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.toupper");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_STR(string_toupper(a)));
	return 0;
}

int str_tolower(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.tolower");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_STR(string_tolower(a)));
	return 0;
}

int str_isalnum(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.isalnum");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_BOOL(string_isalnum(a)));
	return 0;
}

int str_isal(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.isal");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_BOOL(string_isal(a)));
	return 0;
}

int str_isnum(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.isnum");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_BOOL(string_isnum(a)));
	return 0;
}

int str_isspace(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.isspace");
	String_t *a = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_BOOL(string_isspace(a)));
	return 0;
}

int str_startswith(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.startswith");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.startswith");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_BOOL(string_startswith(haystack, needle)));
	return 0;
}

int str_endswith(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.endswith");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.endswith");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_BOOL(string_endswith(haystack, needle)));
	return 0;
}

int str_replace(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.replace");
	String_t *replace_str = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.replace");
	String_t *search_str = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.replace");
	String_t *str = YASL_GETSTR(vm_pop((struct VM *)S));

	if (yasl_string_len(search_str) < 1) {
		printf("Error: str.replace(...) expected search string with length at least 1\n");
		return -1;
	}

	VM_PUSH((struct VM *)S, YASL_STR(string_replace(str, search_str, replace_str)));
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

	vm_pushint((struct VM *)S, string_count(haystack, needle));
	return 0;
}

static int str_split_default(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.split");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_LIST(string_split_default(haystack)));
	return 0;
}

int str_split(struct YASL_State *S) {
	if (YASL_ISUNDEF(vm_peek((struct VM *)S))) {
		vm_pop((struct VM *)S);
		return str_split_default(S);
	}
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.split");
	String_t *needle = YASL_GETSTR(vm_pop((struct VM *)S));
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.split");
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	if (yasl_string_len(needle) == 0) {
		printf("Error: str.split(...) requires type %x of length > 0 as second argument\n", Y_STR);
		return -1;
	}

	VM_PUSH((struct VM *)S, YASL_LIST(string_split(haystack, needle)));
	return 0;
}

static int str_ltrim_default(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.trim");
	struct YASL_Object top = vm_peek((struct VM *)S);
	inc_ref(&top);
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_STR(string_ltrim_default(haystack)));
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

        VM_PUSH((struct VM *)S,
                YASL_STR(string_ltrim(haystack, needle)));

    return 0;
}

static int str_rtrim_default(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.trim");
	struct YASL_Object top = vm_peek((struct VM *)S);
	inc_ref(&top);
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_STR(string_rtrim_default(haystack)));
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

	VM_PUSH((struct VM *)S, YASL_STR(string_rtrim(haystack, needle)));
	return 0;
}

static int str_trim_default(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_STR, "str.trim");
	struct YASL_Object top = vm_peek((struct VM *)S);
	inc_ref(&top);
	String_t *haystack = YASL_GETSTR(vm_pop((struct VM *)S));

	VM_PUSH((struct VM *)S, YASL_STR(string_trim_default(haystack)));
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

	VM_PUSH((struct VM *)S, YASL_STR(string_trim(haystack, needle)));
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

	VM_PUSH((struct VM *)S, YASL_STR(string_rep(string, num)));
	return 0;
}
