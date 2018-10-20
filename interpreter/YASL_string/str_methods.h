#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../list/list.h"

int str___get(struct VM *vm);

int str_contains(struct VM *vm);

int str_tobool(struct VM *vm);

int str_tostr(struct VM *vm);

int str_tofloat64(struct VM *vm);

int str_toupper(struct VM *vm);

int str_tolower(struct VM *vm);

int str_isalnum(struct VM* vm);

int str_isal(struct VM* vm);

int str_isnum(struct VM* vm);

int str_isspace(struct VM* vm);

int str_startswith(struct VM* vm);

int str_endswith(struct VM* vm);

int str_replace(struct VM* vm);

int str_search(struct VM* vm);

int str_split(struct VM* vm);

int str_ltrim(struct VM *vm);

int str_rtrim(struct VM *vm);

int str_trim(struct VM *vm);
