#include "undef_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"
#include "yasl_types.h"

int undef_tostr(struct YASL_State *S) {
	if (!YASL_isundef(S)) {
		YASL_print_err_bad_arg_type(S, "undef.tostr", 0, "undef", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	YASL_pop(S);
	YASL_pushlitszstring(S, "undef");
	return YASL_SUCCESS;
}
