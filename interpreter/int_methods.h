#ifndef YASL_INT_METHODS_H_
#define YASL_INT_METHODS_H_

struct YASL_State;

void int_toint(struct YASL_State *S);

void int_tobool(struct YASL_State *S);

void int_tofloat(struct YASL_State *S);

void int_tostr(struct YASL_State *S);

#endif
