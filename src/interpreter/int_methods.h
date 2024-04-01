#ifndef YASL_INT_METHODS_H_
#define YASL_INT_METHODS_H_

struct YASL_State;

int int_toint(struct YASL_State *S);

int int_tobool(struct YASL_State *S);

int int_tofloat(struct YASL_State *S);

int int_tostr(struct YASL_State *S);

int int_tochar(struct YASL_State *S);

#endif
