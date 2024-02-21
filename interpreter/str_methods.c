#include "str_methods.h"

#include <stddef.h>
#include <ctype.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"
#include "yasl_types.h"
#include "yasl_state.h"

#undef min

static size_t min(size_t a, size_t b) {
	return a < b ? a : b;
}

static struct YASL_String *checkstr(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnstr(S, pos)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, pos, YASL_STR_NAME);
	}

	return vm_peekstr((struct VM *)S, ((struct VM *)S)->fp + 1 + pos);
}

int str___get(struct YASL_State *S) {
	struct YASL_String *str = checkstr(S, "str.__get", 0);
	yasl_int index = YASLX_checknint(S, "str.__get", 1);

	if (index < -(yasl_int) YASL_String_len(str) ||
		   index >= (yasl_int) YASL_String_len(
			   str)) {
		YASLX_print_and_throw_err_value(S, "unable to index str of length %" PRI_SIZET " with index %ld.", YASL_String_len(str), (long)index);
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
	struct YASL_String *str = checkstr(S, "str.__len", 0);
	YASL_pushint(S, YASL_String_len(str));
	return 1;
}

static int str___next(struct YASL_State *S) {
	size_t len;
	const char *str = YASLX_checknstr(S, "str.__next", 0, &len);
	yasl_int curr = YASLX_checknint(S, "str.__next", 1);

	if (curr < -(yasl_int)len || curr >= (yasl_int)len) {
		YASL_pushbool(S, false);
		return 1;
	}

	YASL_pushint(S, curr + 1);
	YASL_pushlstr(S, str + curr, 1);
	YASL_pushbool(S, true);
	return 3;
}

int str___iter(struct YASL_State *S) {
	checkstr(S, "str.__iter", 0);
	YASL_pushcfunction(S, &str___next, 2);
	YASL_pushint(S, 0);
	return 2;
}

int str_tobool(struct YASL_State* S) {
	struct YASL_String *a = checkstr(S, "str.tobool", 0);
	YASL_pushbool(S, YASL_String_len(a) != 0);
	return 1;
}

int str_tofloat(struct YASL_State *S) {
	struct YASL_String *str = checkstr(S, "str.tofloat", 0);
	YASL_pushfloat(S, YASL_String_tofloat(str));
	return 1;
}

int str_toint(struct YASL_State *S) {
	struct YASL_String *str = checkstr(S, "str.toint", 0);
	YASL_pushint(S, YASL_String_toint(str));
	return 1;
}

int str_tolist(struct YASL_State *S) {
	struct YASL_String *str = checkstr(S, "str.tolist", 0);
	yasl_int n = YASLX_checknoptint(S, "str.tolist", 1, 1);
	if (n <= 0) {
		YASLX_print_and_throw_err_value(S, "Expected a positive number, got: %" PRId64 ".", n);
	}
	size_t size = n;
	const size_t len = YASL_String_len(str);
	YASL_pushlist(S);
	for (size_t i = 0; i < len; i+= size) {
		YASL_pushlstr(S, YASL_String_chars(str) + i, min(size, len - i));
		YASL_listpush(S);
	}
	return 1;
}

int str_spread(struct YASL_State *S) {
	size_t len;
	const char *str = YASLX_checknstr(S, "str.spread", 0, &len);
	for (size_t i = 0; i < len; i++) {
		YASL_pushlstr(S, str + i, 1);
	}
	return (int)len;
}

int str_tostr(struct YASL_State *S) {
	struct YASL_String *str = checkstr(S, "str.tostr", 0);
	if (YASL_isnundef(S, 1)) {
		YASL_pop(S);
		return 1;
	}

	size_t len;
	const char *fmt = YASLX_checknstr(S, "str.tostr", 1, &len);
	// struct YASL_String *format =  checkstr(S, "str.tostr", 1);
	if (len != 1) {
		YASLX_print_and_throw_err_value(S, "Expected str of len 1, got str of len %" PRI_SIZET ".", len);
	}

	if (*fmt != 'r') {
		YASLX_print_and_throw_err_value(S, "Unexpected format str: '%c'.", *fmt);
	}

	const char *str_chars = YASL_String_chars(str);
	size_t buffer_size = YASL_String_len(str) + 2;
	char *buffer = (char *)malloc(buffer_size);
	size_t curr = 0;
	buffer[curr++] = '\'';
	for (size_t i = 0; i < YASL_String_len(str); i++, curr++) {
		const unsigned char c = (unsigned char)str_chars[i];
		switch (c) {
#define X(escape, c) case escape: buffer = (char *)realloc(buffer, ++buffer_size); buffer[curr++] = '\\'; buffer[curr] = c; continue;
#include "escapes.x"
#undef X
		default:
			break;
		}

		if (!isprint(c)) {
			char tmp[3] = { '0', '0', 0x00 };
			sprintf(tmp + (c < 16), "%x", c);
			buffer_size += 3;
			buffer = (char *)realloc(buffer, buffer_size);
			buffer[curr++] = '\\';
			buffer[curr++] = 'x';
			memcpy(buffer + curr, tmp, 2);
			curr += 1;
			continue;
		}

		if (c == '\'') {
			buffer = (char *)realloc(buffer, ++buffer_size);
			buffer[curr++] = '\\';
		}
		buffer[curr] = c;
	}
	buffer[curr++] = '\'';

	YASL_pushlstr(S, buffer, curr);
	free(buffer);
	return 1;
}

int str_toupper(struct YASL_State *S) {
	struct YASL_String *a = checkstr(S, "str.toupper", 0);
	vm_push((struct VM *) S, YASL_STR(YASL_String_toupper(a)));
	return 1;
}

int str_tolower(struct YASL_State *S) {
	struct YASL_String *a = checkstr(S, "str.tolower", 0);
	vm_push((struct VM *) S, YASL_STR(YASL_String_tolower(a)));
	return 1;
}

/* Iterates through the string and checks each character against a predicate. */
#define DEFINE_STR_IS_X(name, fun) static bool YASL_String_##name(const char *str, const size_t len) {\
	size_t i = 0;\
	char curr;\
	while (i < len) {\
		curr = str[i++];\
		if (!fun(curr)) \
			return false;\
	}\
	return true;\
}

DEFINE_STR_IS_X(isal, isalpha);
DEFINE_STR_IS_X(isnum, isdigit);
DEFINE_STR_IS_X(isalnum, isalnum);
DEFINE_STR_IS_X(isprint, isprint);
DEFINE_STR_IS_X(isspace, iswhitespace);

#define DEFINE_STR_IS_X_FN(name) \
int str_##name(struct YASL_State *S) {\
	size_t len;\
	const char *str = YASLX_checknstr(S, "str." #name, 0, &len);\
	YASL_pushbool(S, YASL_String_##name(str, len));\
	return 1;\
}

DEFINE_STR_IS_X_FN(isalnum)
DEFINE_STR_IS_X_FN(isal)
DEFINE_STR_IS_X_FN(isnum)
DEFINE_STR_IS_X_FN(isprint)
DEFINE_STR_IS_X_FN(isspace)

int str_tobyte(struct YASL_State *S) {
	size_t len;
	const char *s = YASLX_checknstr(S, "str.tobyte", 0, &len);
	if (len != 1) {
		YASLX_print_and_throw_err_value(S, "str.tobyte expected a str of len 1.");
	}
	YASL_pushint(S, *s);
	return 1;
}

int str_startswith(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.startswith", 0);
	struct YASL_String *needle = checkstr(S, "str.startswith", 1);

	YASL_pushbool(S, YASL_String_startswith(haystack, needle));
	return 1;
}

int str_endswith(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.endswith", 0);
	struct YASL_String *needle = checkstr(S, "str.endswith", 1);

	YASL_pushbool(S, YASL_String_endswith(haystack, needle));
	return 1;
}

static int str_replace_default(struct YASL_State *S) {
	struct YASL_String *replace_str = checkstr(S, "str.replace", 2);
	struct YASL_String *search_str = checkstr(S, "str.replace", 1);
	struct YASL_String *str = checkstr(S, "str.replace", 0);

	if (YASL_String_len(search_str) < 1) {
		YASLX_print_and_throw_err_value(S, "str.replace expected a nonempty str as arg 1.");
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
	struct YASL_String *replace_str = checkstr(S, "str.replace", 2);
	struct YASL_String *search_str = checkstr(S, "str.replace", 1);
	struct YASL_String *str = checkstr(S, "str.replace", 0);

	if (YASL_String_len(search_str) < 1) {
		YASLX_print_and_throw_err_value(S, "str.replace expected a nonempty str as arg 1.");
	}

	int replacements = 0;
	vm_push((struct VM *) S, YASL_STR(YASL_String_replace_fast(str, search_str, replace_str, &replacements, max)));
	YASL_pushint(S, replacements);
	return 2;
}

int str_search(struct YASL_State *S) {
	struct YASL_String *needle = checkstr(S, "str.search", 1);
	struct YASL_String *haystack = checkstr(S, "str.search", 0);

	int64_t index = str_find_index(haystack, needle);
	if (index != -1) YASL_pushint(S, index);
	else YASL_pushundef(S);

	return 1;
}

int str_count(struct YASL_State *S) {
	struct YASL_String *needle = checkstr(S, "str.count", 1);
	struct YASL_String *haystack = checkstr(S, "str.count", 0);
	YASL_pushint(S, YASL_String_count(haystack, needle));
	return 1;
}

static void str_split_max(struct YASL_State *S, yasl_int max_splits) {
	struct YASL_String *needle = checkstr(S, "str.split", 1);
	struct YASL_String *haystack = checkstr(S, "str.split", 0);

	if (max_splits < 0) {
		YASLX_print_and_throw_err_value(S, "str.split expected a non-negative int as arg 2.");
	}

	struct RC_UserData *result = rcls_new();
	ud_setmt(&S->vm, result, (&S->vm)->builtins_htable[Y_LIST]);
	YASL_String_split_max_fast((struct YASL_List *)result->data, haystack, needle, max_splits);
	vm_push((struct VM *) S, YASL_LIST(result));
}

static void str_split_default(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.split", 0);
	struct RC_UserData *result = rcls_new();
	ud_setmt(&S->vm, result, (&S->vm)->builtins_htable[Y_LIST]);

	YASL_String_split_default((struct YASL_List *)result->data, haystack);
	vm_push((struct VM *) S, YASL_LIST(result));
}

static void str_split_default_max(struct YASL_State *S, yasl_int max_splits) {
	struct YASL_String *haystack = checkstr(S, "str.split", 0);
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
		YASLX_print_err_bad_arg_type_n(S, "str.split", 2, YASL_INT_NAME " or " YASL_UNDEF_NAME);
		YASLX_throw_err_type(S);
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

	struct YASL_String *needle = checkstr(S, "str.split", 1);
	struct YASL_String *haystack = checkstr(S, "str.split", 0);

	if (YASL_String_len(needle) == 0) {
		YASLX_print_and_throw_err_value(S, "str.split expected a nonempty str as arg 1.");
	}

	struct RC_UserData *result = rcls_new();
	ud_setmt(&S->vm, result, (&S->vm)->builtins_htable[Y_LIST]);

	YASL_String_split_fast((struct YASL_List *)result->data, haystack, needle);
	vm_push((struct VM *) S, YASL_LIST(result));
	return 1;
}

static void str_ltrim_default(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.ltrim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_ltrim_default(haystack)));
}

int str_ltrim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		str_ltrim_default(S);
		return 1;
	}

	struct YASL_String *needle = checkstr(S, "str.ltrim", 1);
	struct YASL_String *haystack = checkstr(S, "str.ltrim", 0);

	vm_push((struct VM *) S,
		YASL_STR(YASL_String_ltrim(haystack, needle)));
	return 1;
}

static void str_rtrim_default(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.rtrim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_rtrim_default(haystack)));
}

int str_rtrim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		str_rtrim_default(S);
		return 1;
	}

	struct YASL_String *needle = checkstr(S, "str.rtrim", 1);
	struct YASL_String *haystack = checkstr(S, "str.rtrim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_rtrim(haystack, needle)));
	return 1;
}

static void str_trim_default(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.trim", 0);
	vm_push((struct VM *) S, YASL_STR(YASL_String_trim_default(haystack)));
}

int str_trim(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		str_trim_default(S);
		return 1;
	}

	struct YASL_String *needle = checkstr(S, "str.trim", 1);
	struct YASL_String *haystack = checkstr(S, "str.trim", 0);

	vm_push((struct VM *) S, YASL_STR(YASL_String_trim(haystack, needle)));
	return 1;
}

int str_repeat(struct YASL_State *S) {
	yasl_int num = YASLX_checknint(S, "str.rep", 1);
	struct YASL_String *string = checkstr(S, "str.rep", 0);

	if (num < 0) {
		YASLX_print_and_throw_err_value(S, "str.rep expected non-negative int as arg 1.");
	}

	vm_push((struct VM *) S, YASL_STR(YASL_String_rep_fast(string, num)));
	return 1;
}
