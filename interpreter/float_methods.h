#ifndef YASL_FLOAT_METHODS_H_
#define YASL_FLOAT_METHODS_H_

struct YASL_State;

int float_toint(struct YASL_State *S);

int float_tobool(struct YASL_State *S);

int float_tofloat(struct YASL_State *S);

int float_tostr(struct YASL_State *S);

#endif
