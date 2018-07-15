#pragma once

#include <stdio.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../../hashtable/hashtable.h"
#include "../list/list.h"

int table___get(VM *vm);

int table___set(VM *vm);

int table_keys(VM *vm);

int table_values(VM *vm);

int table_clone(VM *vm);