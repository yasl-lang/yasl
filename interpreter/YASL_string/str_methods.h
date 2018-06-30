#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../list/list.h"

int str___get(VM *vm);

int str_tobool(VM *vm);

int str_tostr(VM *vm);

int str_toupper(VM *vm);

int str_tolower(VM *vm);

int str_isalnum(VM* vm);

int str_isal(VM* vm);

int str_isnum(VM* vm);

int str_isspace(VM* vm);

int str_startswith(VM* vm);

int str_endswith(VM* vm);

int str_search(VM* vm);

int str_split(VM* vm);

int str_ltrim(VM *vm);

int str_rtrim(VM *vm);

int str_trim(VM *vm);