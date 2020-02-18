#include "upvalue.h"

void vm_insert_upval(struct VM *const vm, struct Upvalue *const upval) {
	if (vm->pending == NULL) {
		vm->pending = upval;
		return;
	}

	if (vm->pending->location < upval->location) {
	    upval->next = vm->pending;
	    vm->pending = upval;
	    return;
	}

	struct Upvalue *prev = NULL;
	struct Upvalue *curr = vm->pending;
	while (curr && curr->location > upval->location) {
		prev = curr;
		curr = curr->next;
	}
	prev->next = upval;
	upval->next = curr;
}

static void upval_close(struct Upvalue *const upval) {
	upval->closed = UPVAL_GET(upval);
	upval->location = &upval->closed;
}

