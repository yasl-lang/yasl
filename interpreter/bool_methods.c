#include "bool_methods.h"

#include "yasl.h"
#include "yasl_float.h"
#include "yasl_include.h"
#include "yasl_types.h"
#include "interpreter/VM.h"

int bool_tostr(struct YASL_State *S) {
	if (!YASL_top_isboolean(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "bool.tostr", 0, Y_BOOL, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	const char *str = YASL_top_popboolean(S) ? "true" : "false";
	YASL_pushlitszstring(S, str);
	return YASL_SUCCESS;
}

int bool_tobool(struct YASL_State *S) {
	if (!YASL_top_isboolean(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "bool.tobool", 0, Y_BOOL, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}
