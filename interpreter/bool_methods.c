#include "bool_methods.h"

#include "VM.h"
#include "YASL_Object.h"
#include "yasl_state.h"

int bool_tostr(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *) S, Y_BOOL, "bool.tostr");
	bool val = (bool)YASL_GETBOOL(vm_pop((struct VM *) S));
	struct YASL_String *string;
	if (val == 0) {
		string = YASL_String_new_sized(strlen("false"), "false");
	} else {
		string = YASL_String_new_sized(strlen("true"), "true");
	}
	struct YASL_Object to = YASL_STR(string);
	vm_push((struct VM *) S, to);
	return 0;
}

int bool_tobool(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_BOOL, "bool.tobool");
	return 0;
}
