#include "int_methods.h"

#include "yasl_state.h"
#include "yasl_error.h

int int_toint(struct YASL_State *S) {
	if (!YASL_ISINT(vm_peek((struct VM *)S))) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("int.toint", 0, Y_INT, vm_peek((struct VM *)S).type);
		return YASL_TYPE_ERROR;
	}
	return 0;
}

int int_tofloat(struct YASL_State *S) {
	if (!YASL_ISINT(vm_peek((struct VM *)S))) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("int.tofloat", 0, Y_INT, vm_peek((struct VM *)S).type);
		return YASL_TYPE_ERROR;
	}
	vm_pushfloat((struct VM *) S, (yasl_float) YASL_GETINT(vm_pop((struct VM *) S)));
	return 0;
}

int int_tostr(struct YASL_State *S) {
	if (!YASL_ISINT(vm_peek((struct VM *)S))) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("int.tostr", 0, Y_INT, vm_peek((struct VM *)S).type);
		return YASL_TYPE_ERROR;
	}
	yasl_int val = YASL_GETINT(vm_pop((struct VM *) S));
	char *ptr = (char *)malloc(snprintf(NULL, 0, "%" PRId64 "", val) + 1);
	sprintf(ptr, "%" PRId64 "", val);
	struct YASL_String *string = YASL_String_new_sized_heap(0, snprintf(NULL, 0, "%" PRId64 "", val), ptr);
	VM_PUSH((struct VM *) S, YASL_STR(string));
	return 0;
}
