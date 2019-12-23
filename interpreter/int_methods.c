#include "int_methods.h"

#include "yasl.h"
#include "yasl_include.h"
#include "yasl_types.h"
#include "VM.h"

int int_toint(struct YASL_State *S) {
	if (!YASL_top_isinteger(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "int.toint", 0, Y_INT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int int_tobool(struct YASL_State *S) {
	if (!YASL_top_isinteger(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "int.tobool", 0, Y_INT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	YASL_pushboolean(S, true);
	return YASL_SUCCESS;
}

int int_tofloat(struct YASL_State *S) {
	if (!YASL_top_isinteger(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "int.tofloat", 0, Y_INT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	YASL_pushfloat(S, (yasl_float) YASL_top_popinteger(S));
	return YASL_SUCCESS;
}

int int_tostr(struct YASL_State *S) {
	if (!YASL_top_isinteger(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "int.tostr", 0, Y_INT, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}
	yasl_int val = YASL_top_popinteger(S);
	int len = snprintf(NULL, 0, "%" PRId64 "", val);
	char *ptr = (char *)malloc(len + 1);
	sprintf(ptr, "%" PRId64 "", val);
	YASL_pushstring(S, ptr, len);
	return YASL_SUCCESS;
}
