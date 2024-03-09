
#include "yasl.h"
#include "yasl_aux.h"
#include "interpreter/VM.h"

void vm_init_buf(struct VM *vm);
void vm_deinit_buf(struct VM *vm);
void vm_CALL_now(struct VM *const vm);

static int yasl_try(struct YASL_State *S) {
	struct VM *vm = (struct VM *)S;
	jmp_buf *old_buf = vm->buf;
	vm->buf = NULL;
	vm_init_buf(vm);
	if (setjmp(*vm->buf)) {
		vm_deinit_buf(vm);
		vm->buf = old_buf;
		YASL_pushbool(S, false);
		return 1;
	}
	printf("vm->fp %d\n", vm->fp + 1);
	vm_INIT_CALL_offset(vm, vm->fp + 1, -1);
	vm_CALL_now(vm);

	vm_deinit_buf(vm);
	vm->buf = old_buf;
	YASL_pushbool(S, true);
	return 1;
}

int YASL_decllib_try(struct YASL_State *S) {

	YASL_pushcfunction(S, &yasl_try, -2);
	YASLX_initglobal(S, "try");
	return YASL_SUCCESS;
}