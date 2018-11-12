#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"


int int64_tofloat64(struct YASL_State *S);

int int64_tostr(struct YASL_State *S);
