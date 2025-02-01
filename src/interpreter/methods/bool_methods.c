#include "bool_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"

int bool_tobool(struct YASL_State *S) {
	YASL_UNUSED(YASLX_checknbool(S, "bool.tobool", 0));
	return 1;
}

int bool_tostr(struct YASL_State *S) {
	bool result = YASLX_checknbool(S, "bool.tostr", 0);
	char format_char = 'r';
	if (!YASL_isundef(S)) {
		size_t len;
		const char *str = YASLX_checknstr(S, "bool.tostr", 1, &len);
		if (len != 1) {
			YASLX_print_and_throw_err_value(S,
							"bool.tostr expected str arg of len 1 in position 1, got str of len %"
								PRI_SIZET
								" ('%s').", len, str);
		}
		format_char = *str;
	}
	switch (format_char) {
	case 'b':
		YASL_pushlit(S, result ? "1" : "0");
		return 1;
	case 'r':
		YASL_pushlit(S, result ? "true" : "false");
		return 1;
	default:
		YASLX_print_and_throw_err_value(S, "Unexpected format str for bool.tostr: '%c'.", format_char);
	}
}

int bool_tobyte(struct YASL_State *S) {
	bool result = YASLX_checknbool(S, "bool.tobyte", 0);
	YASL_pushint(S, result);
	return 1;
}

int bool___bor(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bor", 0);
	bool right = YASLX_checknbool(S, "bool.__bor", 1);

	YASL_pushbool(S, left || right);
	return 1;
}

int bool___band(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__band", 0);
	bool right = YASLX_checknbool(S, "bool.__band", 1);

	YASL_pushbool(S, left && right);
	return 1;
}

int bool___bandnot(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bandnot", 0);
	bool right = YASLX_checknbool(S, "bool.__bandnot", 1);

	YASL_pushbool(S, left && !right);
	return 1;
}

int bool___bxor(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bxor", 0);
	bool right = YASLX_checknbool(S, "bool.__bxor", 1);

	YASL_pushbool(S, left ^ right);
	return 1;
}

int bool___bnot(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bnot", 0);

	YASL_pushbool(S, !left);
	return 1;
}
