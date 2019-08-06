#include "undef_methods.h"

#include "yasl_error.h"
#include "yasl_state.h"

int undef_tostr(struct YASL_State *S) {
	if (!YASL_ISUNDEF(vm_peek((struct VM *) S))) {
		YASL_PRINT_ERROR_BAD_ARG_TYPE("undef.tostr", 0, Y_UNDEF, vm_peek((struct VM *) S).type);
		return YASL_TYPE_ERROR;
	}
	vm_pop((struct VM *) S);
	VM_PUSH((struct VM *) S, YASL_STR(YASL_String_new_sized(strlen("undef"), "undef")));
	return YASL_SUCCESS;
}
