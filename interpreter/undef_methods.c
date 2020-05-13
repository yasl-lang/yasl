#include "undef_methods.h"

#include "yasl.h"
#include "yasl_include.h"
#include "yasl_types.h"
#include "VM.h"

int undef_tostr(struct YASL_State *S) {
	if (!YASL_isundef(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "undef.tostr", 0, Y_UNDEF, YASL_peektype(S));
		return YASL_TYPE_ERROR;
	}
	YASL_pop(S);
	YASL_pushlitszstring(S, "undef");
	return YASL_SUCCESS;
}
