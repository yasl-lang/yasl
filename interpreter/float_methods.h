#ifndef YASL_FLOAT_METHODS_H_
#define YASL_FLOAT_METHODS_H_

struct YASL_State;

void float_toint(struct YASL_State *S);

void float_tobool(struct YASL_State *S);

void float_tofloat(struct YASL_State *S);

void float_tostr(struct YASL_State *S);

#endif
