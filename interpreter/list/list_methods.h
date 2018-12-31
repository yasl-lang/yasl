#pragma once

struct YASL_State;

int list___get(struct YASL_State *S);

int list___set(struct YASL_State *S);

int list_tostr(struct YASL_State *S);

int list_push(struct YASL_State *S);

int list_copy(struct YASL_State *S);

int list_extend(struct YASL_State *S);

int list_pop(struct YASL_State *S);

int list_search(struct YASL_State *S);

int list_reverse(struct YASL_State *S);

int list_slice(struct YASL_State *S);

int list_clear(struct YASL_State *S);

int list_join(struct YASL_State *S);