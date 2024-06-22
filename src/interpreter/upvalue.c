#include "upvalue.h"

struct Upvalue *upval_new(struct YASL_Object *const location) {
	struct Upvalue *upval = (struct Upvalue *)malloc(sizeof(struct Upvalue));
	upval->rc = NEW_RC();
	upval->location = location;
	upval->next = NULL;
	return upval;
}

struct YASL_Object upval_get(const struct Upvalue *const upval) {
	return (*upval->location);
}

void upval_set(struct VM* const vm, struct Upvalue *const upval, const struct YASL_Object v) {
	vm_dec_ref(vm, upval->location);
	*upval->location = v;
}

void upval_close(struct Upvalue *const upval) {
	upval->closed = upval_get(upval);
	upval->location = &upval->closed;
}

