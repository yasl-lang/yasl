#include "undef_methods.h"

#include "interpreter/VM/VM.h"
#include "yasl_state.h"
#include "yasl_conf.h"

int undef_tostr(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_UNDEF, "undef.tostr");
	vm_pop((struct VM *)S);
	vm_push((struct VM *)S, YASL_STR(str_new_sized(strlen("undef"), "undef")));
	return 0;
}
