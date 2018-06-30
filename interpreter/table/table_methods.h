#pragma once

#include <stdio.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../../hashtable/hashtable.h"
#include "../list/list.h"

int map___get(VM *vm);

int map___set(VM *vm);

int map_keys(VM *vm);

int map_values(VM *vm);

int map_clone(VM *vm);