#ifndef YASL_TABLE_METHODS_H_
#define YASL_TABLE_METHODS_H_

struct YASL_State;

int table___len(struct YASL_State *S);

int table___get(struct YASL_State *S);

int table___set(struct YASL_State *S);

int table___bor(struct YASL_State *S);

int table___eq(struct YASL_State *S);

int table_tostr(struct YASL_State *S);

int table_keys(struct YASL_State *S);

int table_values(struct YASL_State *S);

int table_remove(struct YASL_State *S);

int table_clone(struct YASL_State *S);

int table_clear(struct YASL_State *S);

#endif
