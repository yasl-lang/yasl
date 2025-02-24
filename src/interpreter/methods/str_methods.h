#ifndef YASL_STR_METHODS_H_
#define YASL_STR_METHODS_H_

struct YASL_State;

#define X(name, ...) int str_##name(struct YASL_State *S);
#include "str_methods.x"
#undef X


#endif
