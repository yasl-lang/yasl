#include "yasl-std-error.h"

#include "yasl_aux.h"

int YASL_error(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		YASL_print_err(S, "Error");
		YASL_throw_err(S, YASL_ERROR);
	}

	const char *str = YASL_peekcstr(S);
	if (!str) {
		YASLX_print_err_bad_arg_type(S, "error", 0, YASL_STR_NAME, YASL_peekntypename(S, 0));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	YASL_print_err(S, "Error: %s", str);
	YASL_throw_err(S, YASL_ERROR);
}


int YASL_decllib_error(struct YASL_State *S) {
	YASL_declglobal(S, "error");
	YASL_pushcfunction(S, YASL_error, 1);
	YASL_setglobal(S, "error");

	return YASL_SUCCESS;
}
