#ifndef YASL_STR_METHODS_H_
#define YASL_STR_METHODS_H_

struct YASL_State;

int str___len(struct YASL_State *S);

int str___get(struct YASL_State *S);

int str___iter(struct YASL_State *S);

int str_tobool(struct YASL_State *S);

int str_tostr(struct YASL_State *S);

int str_tofloat(struct YASL_State *S);

int str_toint(struct YASL_State *S);

int str_tolist(struct YASL_State *S);

int str_spread(struct YASL_State *S);

int str_toupper(struct YASL_State *S);

int str_tolower(struct YASL_State *S);

int str_isalnum(struct YASL_State *S);

int str_isal(struct YASL_State *S);

int str_isnum(struct YASL_State *S);

int str_isspace(struct YASL_State *S);

int str_tobyte(struct YASL_State *S);

int str_startswith(struct YASL_State *S);

int str_endswith(struct YASL_State *S);

int str_replace(struct YASL_State *S);

int str_search(struct YASL_State *S);

int str_count(struct YASL_State *S);

int str_split(struct YASL_State *S);

int str_ltrim(struct YASL_State *S);

int str_rtrim(struct YASL_State *S);

int str_trim(struct YASL_State *S);

int str_repeat(struct YASL_State *S);

#endif
