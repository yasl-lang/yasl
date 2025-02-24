#ifndef YASL_BOOL_METHODS_H_
#define YASL_BOOL_METHODS_H_

struct YASL_State;

#define X(name, ...) int bool_##name(struct YASL_State *S);
#include "bool_methods.x"
#undef X

#endif
