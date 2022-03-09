#ifndef YASL_CLOSURE_H_
#define YASL_CLOSURE_H_

#include "refcount.h"
#include "upvalue.h"

struct Closure {
	struct RC rc;
	unsigned char *f;
	size_t num_upvalues;
	struct Upvalue *upvalues[];
};

void closure_del_data(struct VM *vm, struct Closure *closure);
void closure_del_rc(struct VM *vm, struct Closure *closure);

#endif
