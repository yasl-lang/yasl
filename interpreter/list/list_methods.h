#pragma once

#include <stdio.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../constant/constant.h"
#include "list.h"


int list___get(VM *vm);

int list___set(VM *vm);

int list_append(VM* vm);
