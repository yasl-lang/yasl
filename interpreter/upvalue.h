#ifndef YASL_UPVALUE_H_
#define YASL_UPVALUE_H_

#include "VM.h"

#define UPVAL_GET(uv) (*(uv)->location)
#define UPVAL_SET(uv, val) (*(uv)->location = val)

struct Upvalue {
    struct YASL_Object *location;
    struct YASL_Object closed;
    struct Upvalue *next;
};

void vm_insert_upval(struct VM *const vm, struct Upvalue *const upval);
void vm_close_all(struct VM *const vm, const int bottom);

struct Closure {
    const unsigned char *f;
    struct Upvalue *upvalues[];
};

#endif
