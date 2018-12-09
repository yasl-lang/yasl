#include "builtins.h"

#include "str_methods.h"
#include "float64_methods.h"
#include "int64_methods.h"
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

Hash_t* float64_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "toint64", &float64_toint64, 1);
    ht_insert_literalcstring_cfunction(ht, "tostr", &float64_tostr, 1);
    return ht;
}


Hash_t* int64_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "tofloat64", &int64_tofloat64, 1);
    ht_insert_literalcstring_cfunction(ht, "tostr", &int64_tostr, 1);
    return ht;
}

Hash_t* bool_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "tostr", &bool_tostr, 1);
    return ht;
}


Hash_t* str_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "tofloat64", &str_tofloat64, 1);
    ht_insert_literalcstring_cfunction(ht, "toint64", &str_toint64, 1);
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

Hash_t* list_builtins() {
    Hash_t* ht = ht_new();
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

Hash_t* table_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_literalcstring_cfunction(ht, "keys", &table_keys, 1);
    ht_insert_literalcstring_cfunction(ht, "values", &table_values, 1);
    ht_insert_literalcstring_cfunction(ht, "copy", &table_clone, 1);
    ht_insert_literalcstring_cfunction(ht, "__get", &table___get, 2);
    ht_insert_literalcstring_cfunction(ht, "__set", &table___set, 3);
    return ht;
}

