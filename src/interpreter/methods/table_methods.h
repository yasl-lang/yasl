#ifndef YASL_TABLE_METHODS_H_
#define YASL_TABLE_METHODS_H_

struct YASL_State;

#define X(name, ...) int table_##name(struct YASL_State *S);
#include "table_methods.x"
#undef X


#endif
