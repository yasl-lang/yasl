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
	struct VM *vm = (struct VM *)S;
	struct YASL_String *str = checkstr(S, "str.__get", 0);
	yasl_int index = YASLX_checknint(S, "str.__get", 1);
	size_t len = YASL_String_len(str);

	if (index < -(yasl_int) len || index >= (yasl_int) len) {
		YASLX_print_and_throw_err_value(S, "unable to index str of length %" PRI_SIZET " with index %ld.", len, (long)index);
	} else {
		if (index >= 0) {
			vm_pushstr(vm, YASL_String_new_substring(vm, str, index, index + 1));
		} else {
			vm_pushstr(vm, YASL_String_new_substring(vm, str, index + len, index + len + 1));
		}
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
	size_t len;
	const char *str = YASLX_checknstr(S, "str.tofloat", 0, &len);
	YASL_pushfloat(S, YASL_String_tofloat(str, len));
	return 1;
}

int str_toint(struct YASL_State *S) {
	size_t len;
	const char *str = YASLX_checknstr(S, "str.toint", 0, &len);
	YASL_pushint(S, YASL_String_toint(str, len));
	return 1;
}

int str_tolist(struct YASL_State *S) {
	struct YASL_String *str = checkstr(S, "str.tolist", 0);
	yasl_int n = YASLX_checknoptint(S, "str.tolist", 1, 1);
	if (n <= 0) {
		YASLX_print_and_throw_err_value(S, "Expected a positive number, got: %" PRId64 ".", n);
	}
	size_t size = (size_t)n;
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
	size_t str_len;
	const char *str_chars = YASLX_checknstr(S, "str.tostr", 0, &str_len);
	if (YASL_isnundef(S, 1)) {
		YASL_pop(S);
		return 1;
	}

	size_t len;
	const char *fmt = YASLX_checknstr(S, "str.tostr", 1, &len);
	if (len != 1) {
		YASLX_print_and_throw_err_value(S, "Expected str of len 1, got str of len %" PRI_SIZET ".", len);
	}

	if (*fmt != 'r') {
		YASLX_print_and_throw_err_value(S, "Unexpected format str: '%c'.", *fmt);
	}

	size_t buffer_size = str_len + 2;
	char *buffer = (char *)malloc(buffer_size);
	size_t curr = 0;
	buffer[curr++] = '\'';
	for (size_t i = 0; i < str_len; i++, curr++) {
		const unsigned char c = (unsigned char)str_chars[i];
		switch (c) {
#define X(escape, c) case escape: buffer = (char *)realloc(buffer, ++buffer_size); buffer[curr++] = '\\'; buffer[curr] = c; continue;
#include "escapes.x"
#undef X
		default:
			break;
		}

		if (!isprint(c)) {
			char tmp[3] = { '0', '0', '\0' };
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
	vm_pushstr((struct VM *) S, (YASL_String_toupper((struct VM *) S, a)));
	return 1;
}

int str_tolower(struct YASL_State *S) {
	struct YASL_String *a = checkstr(S, "str.tolower", 0);
	vm_pushstr((struct VM *) S, (YASL_String_tolower((struct VM *) S, a)));
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
DEFINE_STR_IS_X(islower, islower);
DEFINE_STR_IS_X(isupper, isupper);

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
DEFINE_STR_IS_X_FN(islower)
DEFINE_STR_IS_X_FN(isupper)

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

static int str_replace_default(struct YASL_State *S, struct YASL_String *str, struct YASL_String *search_str, struct YASL_String *replace_str) {
	int replacements = 0;
	vm_pushstr((struct VM *) S, YASL_String_replace_fast_default((struct VM *)S, str, search_str, replace_str, &replacements));
	YASL_pushint(S, replacements);
	return 2;
}

int str_replace(struct YASL_State *S) {
	struct YASL_String *str = checkstr(S, "str.replace", 0);
	struct YASL_String *search_str = checkstr(S, "str.replace", 1);
	if (YASL_String_len(search_str) < 1) {
		YASLX_print_and_throw_err_value(S, "str.replace expected a non-empty str as arg 1.");
	}
	struct YASL_String *replace_str = checkstr(S, "str.replace", 2);

	if (YASL_isnundef(S, 3)) {
		return str_replace_default(S, str, search_str, replace_str);
	}

	yasl_int max = YASLX_checknint(S, "str.replace", 3);

	int replacements = 0;
	vm_pushstr((struct VM *) S, YASL_String_replace_fast((struct VM *)S, str, search_str, replace_str, &replacements, max));
	YASL_pushint(S, replacements);
	return 2;
}

int str_search(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.search", 0);
	struct YASL_String *needle = checkstr(S, "str.search", 1);

	int64_t index = str_find_index(haystack, needle);
	if (index != -1) YASL_pushint(S, index);
	else YASL_pushundef(S);

	return 1;
}

int str_has(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.has", 0);
	struct YASL_String *needle = checkstr(S, "str.has", 1);

	int64_t index = str_find_index(haystack, needle);
	YASL_pushbool(S, index != -1);
	return 1;
}

int str_count(struct YASL_State *S) {
	struct YASL_String *haystack = checkstr(S, "str.count", 0);
	struct YASL_String *needle = checkstr(S, "str.count", 1);
	if (YASL_String_len(needle) == 0) {
		YASL_pushint(S, YASL_String_len(haystack) + 1);
		return 1;
	}
	YASL_pushint(S, YASL_String_count(haystack, needle));
	return 1;
}

static void str_split_max(struct YASL_State *S, struct YASL_String *haystack, yasl_int max_splits) {
	struct VM *vm = (struct VM *)S;
	struct YASL_String *needle = checkstr(S, "str.split", 1);

	if (YASL_String_len(needle) == 0) {
		YASLX_print_and_throw_err_value(S, "str.split expected a non-empty str as arg 1.");
	}

	if (max_splits < 0) {
		YASLX_print_and_throw_err_value(S, "str.split expected a non-negative int as arg 2.");
	}

	struct RC_UserData *result = rcls_new(vm);
	YASL_String_split_max_fast(vm, (struct YASL_List *)result->data, haystack, needle, max_splits);
	vm_pushlist(vm, result);
}

static void str_split_default(struct YASL_State *S, struct YASL_String *haystack) {
	struct VM *vm = (struct VM *)S;
	struct RC_UserData *result = rcls_new(&S->vm);

	YASL_String_split_default(vm, (struct YASL_List *)result->data, haystack);
	vm_pushlist(vm, result);
}

static void str_split_default_max(struct YASL_State *S, struct YASL_String *haystack, yasl_int max_splits) {
	struct VM *vm = (struct VM *)S;
	struct RC_UserData *result = rcls_new(vm);

	if (max_splits == 0) {
		vm_pushlist((struct VM *) S, result);
		vm_pushstr((struct VM *)S, haystack);
		YASL_listpush(S);
	} else {
		YASL_String_split_default_max(vm, (struct YASL_List *)result->data, haystack, max_splits);
		vm_pushlist(vm, result);
	}
}

int str_split(struct YASL_State *S) {
	struct VM *vm = (struct VM  *)S;
	struct YASL_String *haystack = checkstr(S, "str.split", 0);

	// We check for 3 parameters first.
	if (YASL_isnint(S, 2)) {
		yasl_int max_splits = YASL_peeknint(S, 2);
		str_split_max(S, haystack, max_splits);
		return 1;
	}

	if (!YASL_isnundef(S, 2)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, "str.split", 2, YASL_INT_NAME " or " YASL_UNDEF_NAME);
	}

	if (YASL_isnundef(S, 1)) {
		str_split_default(S, haystack);
		return 1;
	}

	if (YASL_isnint(S, 1)) {
		yasl_int max_splits = YASL_peeknint(S, 1);
		str_split_default_max(S, haystack, max_splits);
		return 1;
	}

	struct YASL_String *needle = checkstr(S, "str.split", 1);

	if (YASL_String_len(needle) == 0) {
		YASLX_print_and_throw_err_value(S, "str.split expected a non-empty str as arg 1.");
	}

	struct RC_UserData *result = rcls_new(&S->vm);

	YASL_String_split_fast(vm, (struct YASL_List *)result->data, haystack, needle);
	vm_pushlist(vm, result);
	return 1;
}

#define DEFINE_TRIM_FN(name) \
static void str_##name##_default(struct YASL_State *S) {\
	struct YASL_String *haystack = checkstr(S, "str." #name, 0);\
	vm_pushstr((struct VM *)S, YASL_String_##name##_default((struct VM *)S, haystack));\
}\
\
int str_##name(struct YASL_State *S) {\
	if (YASL_isundef(S)) {\
		str_##name##_default(S);\
		return 1;\
	}\
	\
	struct YASL_String *haystack = checkstr(S, "str." #name, 0);\
	struct YASL_String *needle = checkstr(S, "str." #name, 1);\
	\
	vm_pushstr((struct VM *) S, YASL_String_##name((struct VM *)S, haystack, needle));\
	return 1;\
}

/*
 * We pass the full name here, rather than just the prefix, because MSVC breaks in some cases with
 * empty macro parameters.
 */
DEFINE_TRIM_FN(ltrim)
DEFINE_TRIM_FN(rtrim)
DEFINE_TRIM_FN(trim)

int str_repeat(struct YASL_State *S) {
	struct YASL_String *string = checkstr(S, "str.rep", 0);
	yasl_int num = YASLX_checknint(S, "str.rep", 1);

	if (num < 0) {
		YASLX_print_and_throw_err_value(S, "str.rep expected non-negative int as arg 1.");
	}

	vm_pushstr((struct VM *) S, YASL_String_rep_fast((struct VM *)S, string, num));
	return 1;
}
