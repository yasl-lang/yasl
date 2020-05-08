#include "closure.h"

void closure_del_data(struct Closure *closure) {
	for (size_t i = 0; i < closure->num_upvalues; i++) {
		struct Upvalue *upval = closure->upvalues[i];
		upval->rc->refs--;
		if (upval->rc->refs == 0) {
			rc_del(upval->rc);
			dec_ref(upval->location);
			free(upval);
		}
	}
}

void closure_del_rc(struct Closure *closure) {
	rc_del(closure->rc);
	free(closure);
}
