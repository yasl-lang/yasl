#include "float_methods.h"

#include <string.h>

#include "yasl.h"
#include "yasl_float.h"
#include "yasl_include.h"
#include "yasl_types.h"


int float_toint(struct YASL_State *S) {
	if (!YASL_top_isdouble(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("float.toint", 0, Y_FLOAT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	yasl_float val = YASL_top_popdouble(S);
	YASL_pushinteger(S, (yasl_int)val);
	return YASL_SUCCESS;
}

int float_tobool(struct YASL_State *S) {
	if (!YASL_top_isdouble(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("float.tobool", 0, Y_FLOAT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	yasl_float a = YASL_top_popdouble(S);
	YASL_pushboolean(S, a == a);
	return YASL_SUCCESS;
}


int float_tofloat(struct YASL_State *S) {
	if (!YASL_top_isdouble(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("float.tofloat", 0, Y_FLOAT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int float_tostr(struct YASL_State *S) {
	if (!YASL_top_isdouble(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("float.tostr", 0, Y_FLOAT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	yasl_float val = YASL_top_popdouble(S);
	char *ptr = float64_to_str(val);
	YASL_pushstring(S, ptr, strlen(ptr));
	return YASL_SUCCESS;
}
