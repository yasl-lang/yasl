#ifndef YASL_LIST_METHODS_H_
#define YASL_LIST_METHODS_H_

struct YASL_State;

void list___len(struct YASL_State *S);

void list___get(struct YASL_State *S);

void list___set(struct YASL_State *S);

void list_tostr(struct YASL_State *S);

void list_push(struct YASL_State *S);

void list_copy(struct YASL_State *S);

void list___add(struct YASL_State *S);

void list___eq(struct YASL_State *S);

void list_pop(struct YASL_State *S);

void list_search(struct YASL_State *S);

void list_reverse(struct YASL_State *S);

void list_clear(struct YASL_State *S);

void list_join(struct YASL_State *S);

void list_sort(struct YASL_State *S);

#endif
