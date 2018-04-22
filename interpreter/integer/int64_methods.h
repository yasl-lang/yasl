#pragma once

#include <stdio.h>
#include "../VM.h"
#include "../opcode.h"
#include "../constant/constant.h"

int int64_tofloat64(VM *vm);

int int64_tostr(VM *vm);