#include "builtins.h"

#include "str_methods.h"
#include "float_methods.h"
#include "int_methods.h"
#include "bool_methods.h"
#include "table_methods.h"
#include "list_methods.h"
#include "VM.h"

int yasl_print(struct VM* vm) {
    struct YASL_Object v = vm_pop(vm);
    int return_value = print(v);
    printf("\n");
    return return_value;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                   *
 *                                                                                                                   *
 *                                                 VTABLES                                                           *
 *                                                                                                                   *
 *                                                                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct RC_Table* float_builtins() {
    struct RC_Table* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "toint", &float_toint, 1);
    ht_insert_literalcstring_cfunction(ht, "tostr", &float_tostr, 1);
    return ht;
}


struct RC_Table* int_builtins() {
    struct RC_Table* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "tofloat", &int_tofloat, 1);
    ht_insert_literalcstring_cfunction(ht, "tostr", &int_tostr, 1);
    return ht;
}

struct RC_Table* bool_builtins() {
    struct RC_Table* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "tostr", &bool_tostr, 1);
    return ht;
}


struct RC_Table* str_builtins() {
    struct RC_Table* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "tofloat", &str_tofloat, 1);
    ht_insert_literalcstring_cfunction(ht, "toint", &str_toint, 1);
    ht_insert_literalcstring_cfunction(ht, "isalnum", &str_isalnum, 1);
    ht_insert_literalcstring_cfunction(ht, "isal", &str_isal, 1);
    ht_insert_literalcstring_cfunction(ht, "isnum", &str_isnum, 1);
    ht_insert_literalcstring_cfunction(ht, "isspace", &str_isspace, 1);
    ht_insert_literalcstring_cfunction(ht, "tobool", &str_tobool, 1);
    ht_insert_literalcstring_cfunction(ht, "tostr", &str_tostr, 1);
    ht_insert_literalcstring_cfunction(ht, "toupper", &str_toupper, 1);
    ht_insert_literalcstring_cfunction(ht, "tolower", &str_tolower, 1);
    ht_insert_literalcstring_cfunction(ht, "startswith", &str_startswith, 2);
    ht_insert_literalcstring_cfunction(ht, "endswith", &str_endswith, 2);
    ht_insert_literalcstring_cfunction(ht, "replace", &str_replace, 3);
    ht_insert_literalcstring_cfunction(ht, "search", &str_search, 2);
    ht_insert_literalcstring_cfunction(ht, "slice", &str_slice, 3);
    ht_insert_literalcstring_cfunction(ht, "split", &str_split, 2);
    ht_insert_literalcstring_cfunction(ht, "ltrim", &str_ltrim, 2);
    ht_insert_literalcstring_cfunction(ht, "rtrim", &str_rtrim, 2);
    ht_insert_literalcstring_cfunction(ht, "trim", &str_trim, 2);
    ht_insert_literalcstring_cfunction(ht, "__get", &str___get, 2);
    return ht;
}

struct RC_Table* list_builtins() {
    struct RC_Table* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "push", &list_push, 2);
    ht_insert_literalcstring_cfunction(ht, "copy", &list_copy, 1);
    ht_insert_literalcstring_cfunction(ht, "extend", &list_extend, 2);
    ht_insert_literalcstring_cfunction(ht, "pop", &list_pop, 1);
    ht_insert_literalcstring_cfunction(ht, "__get", &list___get, 2);
    ht_insert_literalcstring_cfunction(ht, "__set", &list___set, 3);
    ht_insert_literalcstring_cfunction(ht, "search", &list_search, 2);
    ht_insert_literalcstring_cfunction(ht, "reverse", &list_reverse, 1);
    return ht;
}

struct RC_Table* table_builtins() {
    struct RC_Table* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "keys", &table_keys, 1);
    ht_insert_literalcstring_cfunction(ht, "values", &table_values, 1);
    ht_insert_literalcstring_cfunction(ht, "copy", &table_clone, 1);
    ht_insert_literalcstring_cfunction(ht, "__get", &table___get, 2);
    ht_insert_literalcstring_cfunction(ht, "__set", &table___set, 3);
    return ht;
}

