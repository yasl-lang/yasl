#ifndef YASL_LIST_METHODS_H_
#define YASL_LIST_METHODS_H_

struct YASL_State;

int list___len(struct YASL_State *S);

int list___get(struct YASL_State *S);

int list___set(struct YASL_State *S);

int list___iter(struct YASL_State *S);

int list_tostr(struct YASL_State *S);

int list_push(struct YASL_State *S);

int list_copy(struct YASL_State *S);

int list___add(struct YASL_State *S);

int list___eq(struct YASL_State *S);

int list_pop(struct YASL_State *S);

int list_search(struct YASL_State *S);

int list_reverse(struct YASL_State *S);

int list_remove(struct YASL_State *S);

int list_clear(struct YASL_State *S);

int list_join(struct YASL_State *S);

int list_sort(struct YASL_State *S);

int list_count(struct YASL_State *S);

int list_spread(struct YASL_State *S);

int list_insert(struct YASL_State *S);

#endif
