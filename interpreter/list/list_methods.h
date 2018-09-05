#pragma once

#include <stdio.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "list.h"


int list___get(VM *vm);

int list___set(VM *vm);

int list_push(VM *vm);

int list_pop(VM* vm);

int list_search(VM *vm);

int list_reverse(VM *vm);