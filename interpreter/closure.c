#include "closure.h"

void closure_del_data(struct Closure *closure) {
	for (size_t i = 0; i < closure->num_upvalues; i++) {
		free(closure->upvalues[i]);
	}
}

void closure_del_rc(struct Closure *closure) {
	rc_del(closure->rc);
	free(closure);
}
