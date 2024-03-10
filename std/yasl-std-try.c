
#include "yasl.h"
#include "yasl_aux.h"
#include "interpreter/VM.h"

void vm_init_buf(struct VM *vm);
void vm_deinit_buf(struct VM *vm);
void vm_CALL_now(struct VM *const vm);
void vm_exitframe_multi(struct VM *const vm, int len);
void vm_rm_range(struct VM *const vm, int start, int end);
void vm_insertbool(struct VM *const vm, int index, bool val);
void vm_rm(struct VM *const vm, int index);

static int yasl_try(struct YASL_State *S) {
	struct VM *vm = (struct VM *)S;
	jmp_buf *old_buf = vm->buf;
	vm->buf = NULL;
	vm_init_buf(vm);
	int old_fp = vm->fp;
	void (*old_print_err)(struct IO *const, const char *const, va_list) = vm->err.print;
	if (setjmp(*vm->buf)) {
		vm_deinit_buf(vm);
		vm->buf = old_buf;
		vm->status = 0;

		while (vm->fp > old_fp) {
			vm_exitframe_multi(vm, 0);
		}

		YASL_pushbool(S, false);
		YASL_loadprinterr(S);
		YASL_resetprinterr(S);
		vm->err.print = old_print_err;
		return 2;
	}
	vm->err.print = &io_print_string;
	int old_sp = vm->fp + 1;
	vm_rm(vm, vm->fp + 2);
	vm_INIT_CALL_offset(vm, vm->fp + 1, -1);
	vm_CALL_now(vm);

	vm->err.print = old_print_err;
	vm_deinit_buf(vm);
	vm->buf = old_buf;
	vm_insertbool(vm, vm->fp + 1, true);
	return vm->sp - old_sp + 1;
}

int YASL_decllib_try(struct YASL_State *S) {

	YASL_pushcfunction(S, &yasl_try, -2);
	YASLX_initglobal(S, "try");
	return YASL_SUCCESS;
}