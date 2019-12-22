#include "str_methods.h"

#include "yasl.h"
#include "yasl_include.h"
#include "yasl_types.h"
#include "yasl_state.h"

int str___get(struct YASL_State *S) {
	if (!YASL_top_isinteger(S)) {
		return -1;
	}
	yasl_int index = YASL_top_popinteger(S);
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.__get", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *str = YASL_GETSTR(vm_pop((struct VM *) S));
	if (index < -(yasl_int) YASL_String_len(str) ||
		   index >= (yasl_int) YASL_String_len(
			   str)) {
		printf("IndexError\n");
		return -1;
	} else {
		if (index >= 0)
			VM_PUSH((struct VM *) S, YASL_STR(
				YASL_String_new_substring(str->start + index,
							  str->start + index + 1,
							  str)));
		else
			VM_PUSH((struct VM *) S,
				YASL_STR(YASL_String_new_substring(
					str->start + index + YASL_String_len(str),
					str->start + index + YASL_String_len(str) + 1,
					str)));
	}
	return YASL_SUCCESS;
}

int str_slice(struct YASL_State *S) {
	struct YASL_Object end_index = vm_pop((struct VM *) S);
	struct YASL_Object start_index = vm_pop((struct VM *) S);
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.__get", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *str = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_ISINT(start_index) || !YASL_ISINT(end_index)) {
		// TODO: clean this up
		return -1;
	} else if (YASL_GETINT(start_index) < -(yasl_int) YASL_String_len(str) ||
		   YASL_GETINT(start_index) > (yasl_int) YASL_String_len(str)) {
		return -1;
	} else if (YASL_GETINT(end_index) < -(yasl_int) YASL_String_len(str) ||
		   YASL_GETINT(end_index) > (yasl_int) YASL_String_len(
			   str)) {
		return -1;
	}

	int64_t start =
		YASL_GETINT(start_index) < 0 ? YASL_GETINT(start_index) + (yasl_int) YASL_String_len(str) : YASL_GETINT(
			start_index);
	int64_t end =
		YASL_GETINT(end_index) < 0 ? YASL_GETINT(end_index) + (yasl_int) YASL_String_len(str) : YASL_GETINT(
			end_index);

	if (start > end) {
		return -1;
	}

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_new_substring(str->start + start, str->start + end, str)));

	return YASL_SUCCESS;
}

int str_tofloat(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.tofloat", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *str = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_FLOAT(YASL_String_tofloat(str)));
	return YASL_SUCCESS;
}

int str_toint(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.toint", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *str = vm_popstr((struct VM *)S);

	YASL_pushinteger(S, YASL_String_toint(str));
	return YASL_SUCCESS;
}

int str_tobool(struct YASL_State* S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.tobool", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));
	YASL_pushboolean(S, YASL_String_len(a) != 0);
	return YASL_SUCCESS;
}

int str_tostr(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.tostr", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int str_toupper(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.toupper", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_toupper(a)));
	return YASL_SUCCESS;
}

int str_tolower(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.tolower", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_tolower(a)));
	return YASL_SUCCESS;
}

int str_isalnum(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.isalnum", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_isalnum(a));
	return YASL_SUCCESS;
}

int str_isal(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.isal", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_isal(a));
	return YASL_SUCCESS;
}

int str_isnum(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.isnum", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_isnum(a));
	return YASL_SUCCESS;
}

int str_isspace(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.isspace", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_isspace(a));
	return YASL_SUCCESS;
}

int str_startswith(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.startswith", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.startswith", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_startswith(haystack, needle));
	return YASL_SUCCESS;
}

int str_endswith(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.endswith", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.endswith", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_endswith(haystack, needle));
	return YASL_SUCCESS;
}

int str_replace(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.replace", 2, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *replace_str = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.replace", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *search_str = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.replace", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *str = YASL_GETSTR(vm_pop((struct VM *) S));

	if (YASL_String_len(search_str) < 1) {
		YASL_PRINT_ERROR(
			"ValueError: %s expected a nonempty str as arg 1.\n",
			"str.replace");
		return YASL_VALUE_ERROR;
	}

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_replace_fast(str, search_str, replace_str)));
	return YASL_SUCCESS;
}

int str_search(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.search", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = vm_popstr((struct VM *) S);
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.search", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = vm_popstr((struct VM *) S);

	int64_t index = str_find_index(haystack, needle);
	if (index != -1) YASL_pushinteger(S, index);
	else
		YASL_pushundef(S);
	return YASL_SUCCESS;
}

int str_count(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.count", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = vm_popstr((struct VM *) S);
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.count", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = vm_popstr((struct VM *) S);

	YASL_pushinteger(S, YASL_String_count(haystack, needle));
	return YASL_SUCCESS;
}

static int str_split_default(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.split", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_LIST(string_split_default(haystack)));
	return YASL_SUCCESS;
}

int str_split(struct YASL_State *S) {
	if (YASL_top_isundef(S)) {
		YASL_popobject(S);
		return str_split_default(S);
	}
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.split", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.split", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	if (YASL_String_len(needle) == 0) {
		YASL_PRINT_ERROR("ValueError: %s expected a nonempty str as arg 1.\n", "str.split");
		return YASL_VALUE_ERROR;
	}

	VM_PUSH((struct VM *) S, YASL_LIST(YASL_String_split_fast(haystack, needle)));
	return YASL_SUCCESS;
}

static int str_ltrim_default(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.ltrim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_ltrim_default(haystack)));
	dec_ref(&top);
	return YASL_SUCCESS;
}

int str_ltrim(struct YASL_State *S) {
	if (YASL_top_isundef(S)) {
		YASL_popobject(S);
		return str_ltrim_default(S);
	}
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.ltrim", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.ltrim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S,
		YASL_STR(YASL_String_ltrim(haystack, needle)));

	return YASL_SUCCESS;
}

static int str_rtrim_default(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.rtrim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_rtrim_default(haystack)));
	dec_ref(&top);
	return YASL_SUCCESS;
}

int str_rtrim(struct YASL_State *S) {
	if (YASL_top_isundef(S)) {
		vm_pop((struct VM *) S);
		return str_rtrim_default(S);
	}
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.rtrim", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.rtrim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_rtrim(haystack, needle)));
	return YASL_SUCCESS;
}

static int str_trim_default(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.trim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_trim_default(haystack)));
	dec_ref(&top);
	return YASL_SUCCESS;
}

int str_trim(struct YASL_State *S) {
	if (YASL_top_isundef(S)) {
		YASL_popobject(S);
		return str_trim_default(S);
	}
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.trim", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.trim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_trim(haystack, needle)));
	dec_ref(&top);
	return YASL_SUCCESS;
}

int str_repeat(struct YASL_State *S) {
	if (!YASL_ISINT(vm_peek((struct VM *) S))) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.rep", 1, Y_INT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	yasl_int num = YASL_GETINT(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("str.rep", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *string = YASL_GETSTR(vm_pop((struct VM *) S));

	if (num < 0) {
		YASL_PRINT_ERROR("ValueError: %s expected non-negative int as arg 1.\n", "str.rep");
		return YASL_VALUE_ERROR;
	}

	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_rep_fast(string, num)));
	return YASL_SUCCESS;
}
