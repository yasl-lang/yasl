#include "str_methods.h"

#include <stddef.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"
#include "yasl_types.h"
#include "yasl_state.h"

#undef min

static size_t min(size_t a, size_t b) {
	return a < b ? a : b;
}

static struct YASL_String *YASLX_checknstr(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnstr(S, pos)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "str", YASL_peekntypename(S, pos));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	return vm_peekstr((struct VM *)S, ((struct VM *)S)->fp + 1 + pos);
}

int str___get(struct YASL_State *S) {
	struct YASL_String *str = YASLX_checknstr(S, "str.__get", 0);
	yasl_int index = YASLX_checknint(S, "str.__get", 1);

	if (index < -(yasl_int) YASL_String_len(str) ||
		   index >= (yasl_int) YASL_String_len(
			   str)) {
		vm_print_err_value(&S->vm, "unable to index str of length %" PRI_SIZET " with index %lld.", YASL_String_len(str), index);
		YASL_throw_err(S, YASL_VALUE_ERROR);
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
	return 1;
}

int str___len(struct YASL_State *S) {
	struct YASL_String *str = YASLX_checknstr(S, "str.__len", 0);
	YASL_pushint(S, YASL_String_len(str));
	return 1;
}

static int str___next(struct YASL_State *S) {
	yasl_int curr = YASLX_checknint(S, "str.__next", 1);
	struct YASL_String *str = YASLX_checknstr(S, "str.__next", 0);

	if (curr < -(yasl_int) YASL_String_len(str) || curr >= (yasl_int)YASL_String_len(str)) {
		YASL_pushbool(S, false);
		return 1;
	}

	YASL_pushint(S, curr + 1);
	vm_pushstr(&S->vm, YASL_String_new_substring(curr, curr + 1, str));
	YASL_pushbool(S, true);
	return 3;
}

int str___iter(struct YASL_State *S) {
	YASLX_checknstr(S, "str.__iter", 0);
	YASL_pushcfunction(S, &str___next, 2);
	YASL_pushint(S, 0);
	return 2;
}

int str_tobool(struct YASL_State* S) {
	struct YASL_String *a = YASLX_checknstr(S, "str.tobool", 0);
	YASL_pushbool(S, YASL_String_len(a) != 0);
	return 1;
}

int str_tofloat(struct YASL_State *S) {
	struct YASL_String *str = YASLX_checknstr(S, "str.tofloat", 0);
	YASL_pushfloat(S, YASL_String_tofloat(str));
	return 1;
}

int str_toint(struct YASL_State *S) {
	struct YASL_String *str = YASLX_checknstr(S, "str.toint", 0);
	YASL_pushint(S, YASL_String_toint(str));
	return 1;
}

int str_tolist(struct YASL_State *S) {
	size_t size = 1;
	if (YASL_isint(S)) {
		yasl_int n = YASL_popint(S);
		if (n <= 0) {
			YASL_print_err(S, "ValueError: Expecte a positive number.");
			YASL_throw_err(S, YASL_VALUE_ERROR);
		}
		size = n;
	} else {
		YASLX_checknundef(S, "str.tolist", 1);
	}
	struct YASL_String *str = YASLX_checknstr(S, "str.tolist", 0);
	const size_t len = YASL_String_len(str);
	YASL_pushlist(S);
	for (size_t i = 0; i < len; i+= size) {
		YASL_pushlstr(S, YASL_String_chars(str) + i, min(size, len - i));
		YASL_listpush(S);
	}
	return 1;
}

int str_spread(struct YASL_State *S) {
	struct YASL_String *str = YASLX_checknstr(S, "str.spread", 0);
	const size_t len = YASL_String_len(str);
	for (size_t i = 0; i < len; i++) {
		YASL_pushlstr(S, YASL_String_chars(str) + i, 1);
	}
	return len;
}

int str_tostr(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		YASLX_print_err_bad_arg_type(S, "str.tostr", 0, "str", YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return 1;
}

int str_toupper(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checknstr(S, "str.toupper", 0);
	vm_push((struct VM *) S, YASL_STR(YASL_String_toupper(a)));
	return 1;
}

int str_tolower(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checknstr(S, "str.tolower", 0);
	vm_push((struct VM *) S, YASL_STR(YASL_String_tolower(a)));
	return 1;
}

int str_isalnum(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checknstr(S, "str.isalnum", 0);
	YASL_pushbool(S, YASL_String_isalnum(a));
	return 1;
}

int str_isal(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checknstr(S, "str.isal", 0);
	YASL_pushbool(S, YASL_String_isal(a));
	return 1;
}

int str_isnum(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checknstr(S, "str.isnum", 0);
	YASL_pushbool(S, YASL_String_isnum(a));
	return 1;
}

int str_isspace(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checknstr(S, "str.isspace", 0);
	YASL_pushbool(S, YASL_String_isspace(a));
	return 1;
}

int str_tobyte(struct YASL_State *S) {
	struct YASL_String *a = YASLX_checknstr(S, "str.tobyte", 0);
	if (YASL_String_len(a) != 1) {
		YASL_print_err(S, MSG_VALUE_ERROR "str.tobyte expected a str of len 1.");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}
	YASL_pushint(S, *YASL_String_chars(a));
	return 1;
}

int str_startswith(struct YASL_State *S) {
	struct YASL_String *haystack = YASLX_checknstr(S, "str.startswith", 0);
	struct YASL_String *needle = YASLX_checknstr(S, "str.startswith", 1);

	YASL_pushbool(S, YASL_String_startswith(haystack, needle));
	return 1;
}

int str_endswith(struct YASL_State *S) {
	struct YASL_String *haystack = YASLX_checknstr(S, "str.endswith", 0);
	struct YASL_String *needle = YASLX_checknstr(S, "str.endswith", 1);

	YASL_pushbool(S, YASL_String_endswith(haystack, needle));
	return 1;
}

static int str_replace_default(struct YASL_State *S) {
	struct YASL_String *replace_str = YASLX_checknstr(S, "str.replace", 2);
	struct YASL_String *search_str = YASLX_checknstr(S, "str.replace", 1);
	struct YASL_String *str = YASLX_checknstr(S, "str.replace", 0);

	if (YASL_String_len(search_str) < 1) {
		vm_print_err_value((struct VM *)S,
			"%s expected a nonempty str as arg 1.",
			"str.replace");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	int replacements = 0;
	vm_push((struct VM *) S, YASL_STR(YASL_String_replace_fast_default(str, search_str, replace_str, &replacements)));
	YASL_pushint(S, replacements);
	return 2;
}

int str_replace(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		return str_replace_default(S);
	}

	yasl_int max = YASLX_checknint(S, "str.replace", 3);
	struct YASL_String *replace_str = YASLX_checknstr(S, "str.replace", 2);
	struct YASL_String *search_str = YASLX_checknstr(S, "str.replace", 1);
	struct YASL_String *str = YASLX_checknstr(S, "str.replace", 0);

	if (YASL_String_len(search_str) < 1) {
		vm_print_err_value((struct VM *)S,
			     "%s expected a nonempty str as arg 1.",
			     "str.replace");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	int replacements = 0;
	vm_push((struct VM *) S, YASL_STR(YASL_String_replace_fast(str, search_str, replace_str, &replacements, max)));
	YASL_pushint(S, replacements);
	return 2;
}

int str_search(struct YASL_State *S) {
	struct YASL_String *needle = YASLX_checknstr(S, "str.search", 1);
	struct YASL_String *haystack = YASLX_checknstr(S, "str.search", 0);

	int64_t index = str_find_index(haystack, needle);
	if (index != -1) YASL_pushint(S, index);
	else YASL_pushundef(S);

	return 1;
}

int str_count(struct YASL_State *S) {
	struct YASL_String *needle = YASLX_checknstr(S, "str.count", 1);
	struct YASL_String *haystack = YASLX_checknstr(S, "str.count", 0);
	YASL_pushint(S, YASL_String_count(haystack, needle));
	return 1;
}

static void str_split_max(struct YASL_State *S, yasl_int max_splits) {
	struct YASL_String *needle = YASLX_checknstr(S, "str.split", 1);
	struct YASL_String *haystack = YASLX_checknstr(S, "str.split", 0);

	if (max_splits < 0) {
		vm_print_err_value((struct VM *)S, "%s expected a non-negative int as arg 2.", "str.split");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	if (max_splits == 0) {
		YASL_pushlist(S);
		return;
	}

	struct RC_UserData *result = rcls_new();
	ud_setmt(&S->vm, result, (&S->vm)->builtins_htable[Y_LIST]);
	YASL_String_split_max_fast((struct YASL_List *)result->data, haystack, needle, max_splits);
	vm_push((struct VM *) S, YASL_LIST(result));
}

static void str_split_default(struct YASL_State *S) {
	struct YASL_String *haystack = YASLX_checknstr(S, "str.split", 0);
	struct RC_UserData *result = rcls_new();
	ud_setmt(&S->vm, result, (&S->vm)->builtins_htable[Y_LIST]);

	YASL_String_split_default((struct YASL_List *)result->data, haystack);
	vm_push((struct VM *) S, YASL_LIST(result));
}

static void str_split_default_max(struct YASL_State *S, yasl_int max_splits) {
	struct YASL_String *haystack = YASLX_checknstr(S, "str.split", 0);
	struct RC_UserData *result = rcls_new();
	ud_setmt(&S->vm, result, (&S->vm)->builtins_htable[Y_LIST]);

	YASL_String_split_default_max((struct YASL_List *)result->data, haystack, max_splits);
	vm_push((struct VM *) S, YASL_LIST(result));
}

int str_split(struct YASL_State *S) {
	if (YASL_isint(S)) {
		yasl_int max_splits = YASL_popint(S);
		str_split_max(S, max_splits);
		return 1;
	}

	if (!YASL_isundef(S)) {
		vm_print_err_type((struct VM *)S, "str.split expected an argument 2 to be of type int or undef, got %s.", YASL_peekntypename(S, 2));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	YASL_pop(S);

	if (YASL_isundef(S)) {
		YASL_pop(S);
		str_split_default(S);
		return 1;
	}

	if (YASL_isint(S)) {
		yasl_int max_splits = YASL_popint(S);
		str_split_default_max(S, max_splits);
		return 1;
	}

	struct YASL_String *needle = YASLX_checknstr(S, "str.split", 1);
	struct YASL_String *haystack = YASLX_checknstr(S, "str.split", 0);

	if (YASL_String_len(needle) == 0) {
		vm_print_err_value((struct VM *)S, "%s expected a nonempty str as arg 1.", "str.split");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	struct RC_UserData *result = rcls_new();
	ud_setmt(&S->vm, result, (&S->vm)->builtins_htable[Y_LIST]);

	YASL_String_split_fast((struct YASL_List *)result->data, haystack, needle);
	vm_push((struct VM *) S, YASL_LIST(result));
	return 1;
}

static void str_ltrim_default(struct YASL_State *S) {
	struct YASL_String *haystack = YASLX_checknstr(S, "str.ltrim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_ltrim_default(haystack)));
}

int str_ltrim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		str_ltrim_default(S);
		return 1;
	}

	struct YASL_String *needle = YASLX_checknstr(S, "str.ltrim", 1);
	struct YASL_String *haystack = YASLX_checknstr(S, "str.ltrim", 0);

	vm_push((struct VM *) S,
		YASL_STR(YASL_String_ltrim(haystack, needle)));
	return 1;
}

static void str_rtrim_default(struct YASL_State *S) {
	struct YASL_String *haystack = YASLX_checknstr(S, "str.rtrim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_rtrim_default(haystack)));
}

int str_rtrim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		str_rtrim_default(S);
		return 1;
	}

	struct YASL_String *needle = YASLX_checknstr(S, "str.rtrim", 1);
	struct YASL_String *haystack = YASLX_checknstr(S, "str.rtrim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_rtrim(haystack, needle)));
	return 1;
}

static void str_trim_default(struct YASL_State *S) {
	struct YASL_String *haystack = YASLX_checknstr(S, "str.trim", 0);
	vm_push((struct VM *) S, YASL_STR(YASL_String_trim_default(haystack)));
}

int str_trim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		str_trim_default(S);
		return 1;
	}

	struct YASL_String *needle = YASLX_checknstr(S, "str.trim", 1);
	struct YASL_String *haystack = YASLX_checknstr(S, "str.trim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_trim(haystack, needle)));
	return 1;
}

int str_repeat(struct YASL_State *S) {
	yasl_int num = YASLX_checknint(S, "str.rep", 1);
	struct YASL_String *string = YASLX_checknstr(S, "str.rep", 0);

	if (num < 0) {
		vm_print_err_value((struct VM *)S, "%s expected non-negative int as arg 1.", "str.rep");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}

	vm_push((struct VM *) S, YASL_STR(YASL_String_rep_fast(string, num)));
	return 1;
}
