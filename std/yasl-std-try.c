
#include "yasl.h"
#include "yasl_aux.h"
#include "interpreter/VM.h"

void vm_init_buf(struct VM *vm);
void vm_deinit_buf(struct VM *vm);
void vm_CALL_now(struct VM *const vm);
void vm_exitframe_multi(struct VM *const vm, int len);
void vm_rm_range(struct VM *const vm, int start, int end);

static int yasl_try(struct YASL_State *S) {
	struct VM *vm = (struct VM *)S;
	jmp_buf *old_buf = vm->buf;
	vm->buf = NULL;
	vm_init_buf(vm);
	int old_fp = vm->fp;
	if (setjmp(*vm->buf)) {
		puts("gasfdsa");
		vm_deinit_buf(vm);
		vm->buf = old_buf;
		vm->status = 0;
		vm->sp = vm->fp + 1;
		// vm_rm_range(vm, vm->fp + 1, vm->sp -1);
		while (vm->fp > old_fp) {
			vm_exitframe_multi(vm, 0);
		}

		YASL_pushbool(S, false);
		return 1;
	}
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