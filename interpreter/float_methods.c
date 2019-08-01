#include "float_methods.h"

#include "yasl_error.h"
#include "yasl_float.h"
#include "yasl_state.h"

int float_toint(struct YASL_State *S) {
	if (!YASL_ISFLOAT(vm_peek((struct VM *)S))) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("float.toint", 0, Y_FLOAT, vm_peek((struct VM *)S).type);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Object a = vm_pop((struct VM *) S);
	vm_push((struct VM *) S, YASL_INT((yasl_int) YASL_GETFLOAT(a)));
	return YASL_SUCCESS;
}

int float_tofloat(struct YASL_State *S) {
	if (!YASL_ISFLOAT(vm_peek((struct VM *)S))) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("float.tofloat", 0, Y_FLOAT, vm_peek((struct VM *)S).type);
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int float_tostr(struct YASL_State *S) {
	if (!YASL_ISFLOAT(vm_peek((struct VM *)S))) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("float.tostr", 0, Y_FLOAT, vm_peek((struct VM *)S).type);
		return YASL_TYPE_ERROR;
	}
	yasl_float val = YASL_GETFLOAT(vm_pop((struct VM *) S));
	char *ptr = float64_to_str(val);
	struct YASL_String *string = YASL_String_new_sized_heap(0, strlen(ptr), ptr);
	struct YASL_Object to = YASL_STR(string);
	vm_push((struct VM *) S, to);
	return YASL_SUCCESS;
}
