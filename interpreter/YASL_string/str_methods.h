#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../list/list.h"

int str___get(struct YASL_State *S);

int str_contains(struct YASL_State *S);

int str_tobool(struct YASL_State *S);

int str_tostr(struct YASL_State *S);

int str_tofloat64(struct YASL_State *S);

int str_toupper(struct YASL_State *S);

int str_tolower(struct YASL_State *S);

int str_isalnum(struct YASL_State *S);

int str_isal(struct YASL_State *S);

int str_isnum(struct YASL_State *S);

int str_isspace(struct YASL_State *S);

int str_startswith(struct YASL_State *S);

int str_endswith(struct YASL_State *S);

int str_replace(struct YASL_State *S);

int str_search(struct YASL_State *S);

int str_split(struct YASL_State *S);

int str_ltrim(struct YASL_State *S);

int str_rtrim(struct YASL_State *S);

int str_trim(struct YASL_State *S);
