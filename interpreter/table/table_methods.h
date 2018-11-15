#pragma once

#include <stdio.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../../hashtable/hashtable.h"
#include "../list/list.h"

int table___get(struct YASL_State *S);

int table___set(struct YASL_State *S);

int table_keys(struct YASL_State *S);

int table_values(struct YASL_State *S);

int table_clone(struct YASL_State *S);