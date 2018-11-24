#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"

int float64_toint64(struct YASL_State *S);

int float64_tostr(struct YASL_State *S);