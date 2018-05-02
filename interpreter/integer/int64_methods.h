#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"

int int64_tofloat64(VM *vm);

int int64_tostr(VM *vm);