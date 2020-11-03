#ifndef YASL_STR_METHODS_H_
#define YASL_STR_METHODS_H_

struct YASL_State;

void str___get(struct YASL_State *S);

void str_tobool(struct YASL_State *S);

void str_tostr(struct YASL_State *S);

void str_tofloat(struct YASL_State *S);

void str_toint(struct YASL_State *S);

void str_toupper(struct YASL_State *S);

void str_tolower(struct YASL_State *S);

void str_isalnum(struct YASL_State *S);

void str_isal(struct YASL_State *S);

void str_isnum(struct YASL_State *S);

void str_isspace(struct YASL_State *S);

void str_startswith(struct YASL_State *S);

void str_endswith(struct YASL_State *S);

void str_replace(struct YASL_State *S);

void str_search(struct YASL_State *S);

void str_count(struct YASL_State *S);

void str_split(struct YASL_State *S);

void str_ltrim(struct YASL_State *S);

void str_rtrim(struct YASL_State *S);

void str_trim(struct YASL_State *S);

void str_repeat(struct YASL_State *S);

#endif
