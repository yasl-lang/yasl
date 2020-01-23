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
		vm_print_err_bad_arg_type((struct VM *)S, "str.__get", 0, Y_STR, YASL_top_peektype(S));
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
			vm_push((struct VM *) S, YASL_STR(
				YASL_String_new_substring(str->start + index,
							  str->start + index + 1,
							  str)));
		else
			vm_push((struct VM *) S,
				YASL_STR(YASL_String_new_substring(
					str->start + index + YASL_String_len(str),
					str->start + index + YASL_String_len(str) + 1,
					str)));
	}
	return YASL_SUCCESS;
}

int str_tofloat(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.tofloat", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *str = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_FLOAT(YASL_String_tofloat(str)));
	return YASL_SUCCESS;
}

int str_toint(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.toint", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *str = vm_popstr((struct VM *)S);

	YASL_pushinteger(S, YASL_String_toint(str));
	return YASL_SUCCESS;
}

int str_tobool(struct YASL_State* S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.tobool", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));
	YASL_pushboolean(S, YASL_String_len(a) != 0);
	return YASL_SUCCESS;
}

int str_tostr(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.tostr", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int str_toupper(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.toupper", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_STR(YASL_String_toupper(a)));
	return YASL_SUCCESS;
}

int str_tolower(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.tolower", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_STR(YASL_String_tolower(a)));
	return YASL_SUCCESS;
}

int str_isalnum(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.isalnum", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_isalnum(a));
	return YASL_SUCCESS;
}

int str_isal(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.isal", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_isal(a));
	return YASL_SUCCESS;
}

int str_isnum(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.isnum", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_isnum(a));
	return YASL_SUCCESS;
}

int str_isspace(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.isspace", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *a = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_isspace(a));
	return YASL_SUCCESS;
}

int str_startswith(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.startswith", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.startswith", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_startswith(haystack, needle));
	return YASL_SUCCESS;
}

int str_endswith(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.endswith", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.endswith", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	YASL_pushboolean(S, YASL_String_endswith(haystack, needle));
	return YASL_SUCCESS;
}

int str_replace(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.replace", 2, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *replace_str = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.replace", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *search_str = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.replace", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *str = YASL_GETSTR(vm_pop((struct VM *) S));

	if (YASL_String_len(search_str) < 1) {
		vm_print_err((struct VM *)S,
			"ValueError: %s expected a nonempty str as arg 1.\n",
			"str.replace");
		return YASL_VALUE_ERROR;
	}

	vm_push((struct VM *) S, YASL_STR(YASL_String_replace_fast(str, search_str, replace_str)));
	return YASL_SUCCESS;
}

int str_search(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.search", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = vm_popstr((struct VM *) S);
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.search", 0, Y_STR, YASL_top_peektype(S));
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
		vm_print_err_bad_arg_type((struct VM *)S, "str.count", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = vm_popstr((struct VM *) S);
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.count", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = vm_popstr((struct VM *) S);

	YASL_pushinteger(S, YASL_String_count(haystack, needle));
	return YASL_SUCCESS;
}

static int str_split_default(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.split", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_LIST(string_split_default(haystack)));
	return YASL_SUCCESS;
}

int str_split(struct YASL_State *S) {
	if (YASL_top_isundef(S)) {
		YASL_pop(S);
		return str_split_default(S);
	}
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.split", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.split", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	if (YASL_String_len(needle) == 0) {
		vm_print_err((struct VM *)S, "ValueError: %s expected a nonempty str as arg 1.\n", "str.split");
		return YASL_VALUE_ERROR;
	}

	vm_push((struct VM *) S, YASL_LIST(YASL_String_split_fast(haystack, needle)));
	return YASL_SUCCESS;
}

static int str_ltrim_default(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.ltrim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_STR(YASL_String_ltrim_default(haystack)));
	dec_ref(&top);
	return YASL_SUCCESS;
}

int str_ltrim(struct YASL_State *S) {
	if (YASL_top_isundef(S)) {
		YASL_pop(S);
		return str_ltrim_default(S);
	}
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.ltrim", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.ltrim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S,
		YASL_STR(YASL_String_ltrim(haystack, needle)));

	return YASL_SUCCESS;
}

static int str_rtrim_default(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.rtrim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_STR(YASL_String_rtrim_default(haystack)));
	dec_ref(&top);
	return YASL_SUCCESS;
}

int str_rtrim(struct YASL_State *S) {
	if (YASL_top_isundef(S)) {
		vm_pop((struct VM *) S);
		return str_rtrim_default(S);
	}
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.rtrim", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.rtrim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_STR(YASL_String_rtrim(haystack, needle)));
	return YASL_SUCCESS;
}

static int str_trim_default(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.trim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_STR(YASL_String_trim_default(haystack)));
	dec_ref(&top);
	return YASL_SUCCESS;
}

int str_trim(struct YASL_State *S) {
	if (YASL_top_isundef(S)) {
		YASL_pop(S);
		return str_trim_default(S);
	}
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.trim", 1, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *needle = YASL_GETSTR(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.trim", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object top = vm_peek((struct VM *) S);
	inc_ref(&top);
	struct YASL_String *haystack = YASL_GETSTR(vm_pop((struct VM *) S));

	vm_push((struct VM *) S, YASL_STR(YASL_String_trim(haystack, needle)));
	dec_ref(&top);
	return YASL_SUCCESS;
}

int str_repeat(struct YASL_State *S) {
	if (!YASL_ISINT(vm_peek((struct VM *) S))) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.rep", 1, Y_INT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	yasl_int num = YASL_GETINT(vm_pop((struct VM *) S));
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "str.rep", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *string = YASL_GETSTR(vm_pop((struct VM *) S));

	if (num < 0) {
		vm_print_err((struct VM *)S, "ValueError: %s expected non-negative int as arg 1.\n", "str.rep");
		return YASL_VALUE_ERROR;
	}

	vm_push((struct VM *) S, YASL_STR(YASL_String_rep_fast(string, num)));
	return YASL_SUCCESS;
}
