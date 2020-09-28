#include "undef_methods.h"

#include "yasl.h"
#include "yasl_aux.h"

int undef_tostr(struct YASL_State *S) {
	if (!YASL_isundef(S)) {
		YASLX_print_err_bad_arg_type(S, "undef.tostr", 0, "undef", YASL_peektypestr(S));
		return YASL_TYPE_ERROR;
	}
	YASL_pop(S);
	YASL_pushlitszstring(S, "undef");
	return YASL_SUCCESS;
}
