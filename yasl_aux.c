#include "yasl_aux.h"
#include "yasl_include.h"

void YASL_print_err_bad_arg_type(struct YASL_State *S,
				 const char *const fn_name,
				 int position,
				 const char *const exp,
				 const char *const act) {
	YASL_print_err(S, MSG_TYPE_ERROR "%s expected arg in position %d to be of type %s, got arg of type %s.\n", fn_name, position, exp, act);
}
