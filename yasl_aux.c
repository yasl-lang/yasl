#include "yasl_aux.h"
#include "yasl_include.h"

void YASLX_print_err_bad_arg_type(struct YASL_State *S,
				 const char *const fn_name,
				 int position,
				 const char *const exp,
				 const char *const act) {
	YASL_print_err(S, MSG_TYPE_ERROR "%s expected arg in position %d to be of type %s, got arg of type %s.", fn_name, position, exp, act);
}

void YASL_print_err_bad_arg_type(struct YASL_State *S,
				  const char *const fn_name,
				  int position,
				  const char *const exp,
				  const char *const act) {
	YASLX_print_err_bad_arg_type(S, fn_name, position, exp, act);
}

int YASLX_decllibs(struct YASL_State *S) {
	YASL_decllib_collections(S);
	YASL_decllib_io(S);
	YASL_decllib_math(S);
	YASL_decllib_require(S);
	YASL_decllib_require_c(S);
	
	return YASL_SUCCESS;
}