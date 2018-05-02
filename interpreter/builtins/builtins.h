#pragma once

#include <stdio.h>
#include "../../methods.h"
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"
#include "../../hashtable/hashtable.h"
#include "../vtable/vtable.h"
#include "../list/list.h"
#include "../YASL_string/YASL_string.h"
#include "../YASL_string/str_methods.h"
#include "../float/float64_methods.h"
#include "../integer/int64_methods.h"
#include "../boolean/bool_methods.h"
#include "../map/map_methods.h"
#include "../file/file_methods.h"
#include "../list/list_methods.h"

typedef int (*Handler)(VM*);

int yasl_print(VM* vm);

int yasl_input(VM* vm);

int yasl_insert(VM* vm);

int yasl_find(VM* vm);

int yasl_keys(VM* vm);

int yasl_values(VM* vm);

int yasl_open(VM* vm);

int yasl_popen(VM *vm);

static const Handler builtins[] = {
    yasl_print,     yasl_input,     yasl_open,      yasl_popen
};

VTable_t* float64_builtins(void);
VTable_t* int64_builtins(void);
VTable_t* str8_builtins(void);
VTable_t* list_builtins(void);
VTable_t* hash_builtins(void);
