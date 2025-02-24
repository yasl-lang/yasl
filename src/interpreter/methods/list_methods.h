#ifndef YASL_LIST_METHODS_H_
#define YASL_LIST_METHODS_H_

struct YASL_State;

#define X(name, ...) int list_##name(struct YASL_State *S);
#include "list_methods.x"
#undef X


#endif
