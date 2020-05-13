#include "float_methods.h"

#include <string.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_float.h"
#include "yasl_include.h"
#include "yasl_types.h"

int float_toint(struct YASL_State *S) {
	if (!YASL_isfloat(S)) {
		YASL_print_err_bad_arg_type(S, "float.toint", 0, "float", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	yasl_float val = YASL_popfloat(S);
	YASL_pushinteger(S, (yasl_int)val);
	return YASL_SUCCESS;
}

int float_tobool(struct YASL_State *S) {
	if (!YASL_isfloat(S)) {
		YASL_print_err_bad_arg_type(S, "float.tobool", 0, "float", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	yasl_float a = YASL_popfloat(S);
	YASL_pushboolean(S, a == a);
	return YASL_SUCCESS;
}


int float_tofloat(struct YASL_State *S) {
	if (!YASL_isfloat(S)) {
		YASL_print_err_bad_arg_type(S, "float.tofloat", 0, "float", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int float_tostr(struct YASL_State *S) {
	if (!YASL_isfloat(S)) {
		YASL_print_err_bad_arg_type(S, "float.tostr", 0, "float", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	yasl_float val = YASL_popfloat(S);
	const char *ptr = float64_to_str(val);
	YASL_pushstring(S, ptr, strlen(ptr));
	return YASL_SUCCESS;
}
