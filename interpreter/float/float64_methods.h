#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM.h"
#include "../opcode.h"
#include "../constant/constant.h"

int float64_toint64(VM *vm);

int float64_tostr(VM *vm);