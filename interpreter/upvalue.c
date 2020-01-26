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

void vm_close_all(struct VM *const vm, const int bottom) {
	struct YASL_Object *tmp = vm->stack + bottom;
	if (vm->pending == NULL) return;
	struct Upvalue *curr = vm->pending;
	while (curr && curr->location > tmp) {
		upval_close(curr);
		curr = curr->next;
	}
	vm->pending = curr;
}
