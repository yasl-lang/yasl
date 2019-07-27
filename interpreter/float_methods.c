#include "float_methods.h"

#include "yasl_float.h"
#include "yasl_state.h"
#include "VM.h"

int float_toint(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *) S, Y_FLOAT, "float.toint64");
	struct YASL_Object a = vm_pop((struct VM *) S);
	vm_push((struct VM *) S, YASL_INT((yasl_int) YASL_GETFLOAT(a)));
	return 0;
}

int float_tofloat(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *) S, Y_FLOAT, "float.toint64");
	return 0;
}

int float_tostr(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *) S, Y_FLOAT, "float.tostr");
	yasl_float val = YASL_GETFLOAT(vm_pop((struct VM *) S));
	char *ptr = float64_to_str(val);
	struct YASL_String *string = YASL_String_new_sized_heap(0, strlen(ptr), ptr);
	struct YASL_Object to = YASL_STR(string);
	vm_push((struct VM *) S, to);
	return 0;
}
