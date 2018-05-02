#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../constant/constant.h"
#include "../list/list.h"

int str___get(VM *vm);

int str_tobool(VM *vm);

int str_tostr(VM *vm);

int str_upcase(VM* vm);

int str_downcase(VM* vm);

int str_isalnum(VM* vm);

int str_isal(VM* vm);

int str_isnum(VM* vm);

int str_isspace(VM* vm);

int str_startswith(VM* vm);

int str_endswith(VM* vm);

int str_search(VM* vm);

int str_split(VM* vm);