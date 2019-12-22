#include "undef_methods.h"

#include "yasl.h"
#include "yasl_include.h"
#include "yasl_types.h"

int undef_tostr(struct YASL_State *S) {
	if (!YASL_top_isundef(S)) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("undef.tostr", 0, Y_UNDEF, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	YASL_popobject(S);
	YASL_pushlitszstring(S, "undef");
	return YASL_SUCCESS;
}
