#include "closure.h"

void closure_del_data(struct VM *vm, struct Closure *closure) {
	for (size_t i = 0; i < closure->num_upvalues; i++) {
		struct Upvalue *upval = closure->upvalues[i];
		upval->rc.refs--;
		if (upval->rc.refs == 0) {
			vm_remove_pending_upvalue(vm, upval);
			if (upval->location == &upval->closed)
				vm_dec_ref(vm, upval->location);
			free(upval);
		}
	}
}

void closure_del_rc(struct VM *vm, struct Closure *closure) {
	vm_free_cyclic(vm, closure);
}
