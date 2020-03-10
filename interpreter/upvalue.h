#ifndef YASL_UPVALUE_H_
#define YASL_UPVALUE_H_

#include "VM.h"
#include "refcount.h"

struct Upvalue {
	struct RC *rc;                 // NOTE: RC MUST BE THE FIRST MEMBER OF THIS STRUCT. DO NOT REARRANGE.
	struct YASL_Object *location;
	struct YASL_Object closed;
	struct Upvalue *next;
};

struct Upvalue *upval_new(struct YASL_Object *const location);
struct YASL_Object upval_get(const struct Upvalue *const upval);
void upval_set(struct Upvalue *const upval, const struct YASL_Object v);

#endif
