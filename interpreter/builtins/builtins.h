#pragma once

#include <stdio.h>
#include "../VM.h"
#include "../opcode.h"
#include "../constant/constant.h"
#include "../hashtable/hashtable.h"
#include "../vtable/vtable.h"
#include "../list/list.h"
#include "../string8/string8.h"
#include "../string8/str_methods.h"
#include "../integer/int64_methods.h"
#include "../hashtable/hash_methods.h"
#include "../file/file_methods.h"


typedef int (*Handler)(VM*);

int yasl_print(VM* vm);

int yasl_input(VM* vm);

int yasl_insert(VM* vm);

int yasl_find(VM* vm);

int yasl_append(VM* vm);

int yasl_keys(VM* vm);

int yasl_values(VM* vm);

int yasl_open(VM* vm);

int yasl_popen(VM *vm);

static const Handler builtins[] = {
    yasl_print,     yasl_insert,    yasl_find,    yasl_append,  yasl_input,     yasl_open,  yasl_popen
};

VTable_t* float64_builtins(void);
VTable_t* int64_builtins(void);
VTable_t* str8_builtins(void);
VTable_t* list_builtins(void);
VTable_t* hash_builtins(void);
