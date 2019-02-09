#include "undef_methods.h"

#include "interpreter/VM/VM.h"
#include "yasl_state.h"
#include "yasl_conf.h"

int undef_tostr(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_UNDEF, "undef.tostr");
	vm_pop(S->vm);
	vm_push(S->vm, YASL_STR(str_new_sized(strlen("undef"), "undef")));
	return 0;
}
