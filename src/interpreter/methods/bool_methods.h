#ifndef YASL_BOOL_METHODS_H_
#define YASL_BOOL_METHODS_H_

struct YASL_State;

int bool_tostr(struct YASL_State *S);

int bool_tobool(struct YASL_State *S);

int bool_tobyte(struct YASL_State *S);

int bool___bor(struct YASL_State *S);

int bool___band(struct YASL_State *S);

int bool___bandnot(struct YASL_State *S);

int bool___bxor(struct YASL_State *S);

int bool___bnot(struct YASL_State *S);

#endif
