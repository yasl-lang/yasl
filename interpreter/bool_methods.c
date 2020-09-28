#include "bool_methods.h"

#include "yasl.h"
#include "yasl_aux.h"

int bool_tostr(struct YASL_State *S) {
	if (!YASL_isbool(S)) {
		YASLX_print_err_bad_arg_type(S, "bool.tostr", 0, "bool", YASL_peektypestr(S));
		return YASL_TYPE_ERROR;
	}
	const char *str = YASL_popbool(S) ? "true" : "false";
	YASL_pushlitszstring(S, str);
	return YASL_SUCCESS;
}

int bool_tobool(struct YASL_State *S) {
	if (!YASL_isbool(S)) {
		YASLX_print_err_bad_arg_type(S, "bool.tobool", 0, "bool", YASL_peektypestr(S));
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}
