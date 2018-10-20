#pragma once

#include <stdio.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../../hashtable/hashtable.h"
#include "../list/list.h"

int table___get(struct VM *vm);

int table___set(struct VM *vm);

int table_keys(struct VM *vm);

int table_values(struct VM *vm);

int table_clone(struct VM *vm);