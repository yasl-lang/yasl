#include "bool_methods.h"

#include "yasl.h"
#include "yasl_float.h"
#include "yasl_include.h"
#include "yasl_types.h"

int bool_tostr(struct YASL_State *S) {
	if (!YASL_top_isboolean(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("bool.tostr", 0, Y_BOOL, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	char *str = YASL_top_popboolean(S) ? "true" : "false";
	YASL_pushlitszstring(S, str);
	return YASL_SUCCESS;
}

int bool_tobool(struct YASL_State *S) {
	if (!YASL_top_isboolean(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("bool.tobool", 0, Y_BOOL, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}
