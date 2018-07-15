#pragma once

#include <stdio.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../../hashtable/hashtable.h"
#include "../list/list.h"
#include "../YASL_string/YASL_string.h"
#include "../YASL_string/str_methods.h"
#include "../float/float64_methods.h"
#include "../integer/int64_methods.h"
#include "../boolean/bool_methods.h"
#include "interpreter/table/table_methods.h"
#include "../file/file_methods.h"
#include "../list/list_methods.h"

int yasl_print(VM* vm);

int yasl_input(VM* vm);

int yasl_open(VM* vm);

int yasl_popen(VM *vm);
