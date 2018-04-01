#pragma once

#include <stdio.h>
#include "../VM.h"
#include "../opcode.h"
#include "../constant/constant.h"
#include "../hashtable/hashtable.h"
#include "../vtable/vtable.h"
#include "../list/list.h"
#include "../string8/string8.h"

typedef int (*Handler)(VM*);

int yasl_print(VM* vm);

int yasl_input(VM* vm);

int yasl_tofloat64(VM* vm);

int yasl_toint64(VM* vm);

int yasl_tobool(VM* vm);

int yasl_tostr8(VM* vm);

int yasl_tolist(VM* vm);

int yasl_tomap(VM* vm);

int yasl_upcase(VM* vm);

int yasl_downcase(VM* vm);

int yasl_isalnum(VM* vm);

int yasl_isal(VM* vm);

int yasl_isnum(VM* vm);

int yasl_isspace(VM* vm);

int yasl_startswith(VM* vm);

int yasl_endswith(VM* vm);

int yasl_search(VM* vm);

int yasl_split(VM* vm);

int yasl_insert(VM* vm);

int yasl_find(VM* vm);

int yasl_append(VM* vm);

int yasl_keys(VM* vm);

int yasl_values(VM* vm);

int yasl_open(VM* vm);

int yasl_close(VM* vm);

int yasl_write(VM* vm);

int yasl_read(VM* vm);

int yasl_readline(VM* vm);

static const Handler builtins[] = {
    yasl_print,     yasl_insert,    yasl_find,    yasl_append,  yasl_input,     yasl_open
};

VTable_t* float64_builtins(void);
VTable_t* int64_builtins(void);
VTable_t* str8_builtins(void);
VTable_t* list_builtins(void);
VTable_t* hash_builtins(void);
