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

#endif
