#ifndef YASL_TABLE_METHODS_H_
#define YASL_TABLE_METHODS_H_

struct YASL_State;

void table___len(struct YASL_State *S);

void table___get(struct YASL_State *S);

void table___set(struct YASL_State *S);

void table___bor(struct YASL_State *S);

void table___eq(struct YASL_State *S);

void table_tostr(struct YASL_State *S);

void table_keys(struct YASL_State *S);

void table_values(struct YASL_State *S);

void table_remove(struct YASL_State *S);

void table_clone(struct YASL_State *S);

void table_clear(struct YASL_State *S);

#endif
