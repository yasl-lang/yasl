#include <inttypes.h>
#include <string.h>
#include <debug.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include <interpreter/YASL_string/YASL_string.h>

#include "builtins.h"

int yasl_print(struct VM* vm) {
    struct YASL_Object v = vm->stack[vm->sp--];    // pop value from top of the stack ...
    if (yasl_type_equals(v.type, Y_LIST)) {
        ls_print(v.value.lval);
        printf("\n");
        return 0;
    } else if (yasl_type_equals(v.type, Y_TABLE)) {
        ht_print(v.value.mval);
        printf("\n");
        return 0;
    }
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
    ht_insert_string_int(ht, "toint64", strlen("toint64"), (int64_t)&float64_toint64);
    ht_insert_string_int(ht, "tostr", strlen("tostr"), (int64_t)&float64_tostr);
    return ht;
}


Hash_t* int64_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_string_int(ht, "tofloat64", strlen("tofloat64"), (int64_t)&int64_tofloat64);
    ht_insert_string_int(ht, "tostr", strlen("tostr"), (int64_t)&int64_tostr);
    return ht;
}

Hash_t* bool_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_string_int(ht, "tostr", strlen("tostr"), (int64_t)&bool_tostr);
    return ht;
}


Hash_t* str_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_string_int(ht, "tofloat64",  strlen("tofloat64"),  (int64_t)&str_tofloat64);
    ht_insert_string_int(ht, "isalnum",    strlen("isalnum"),    (int64_t)&str_isalnum);
    ht_insert_string_int(ht, "isal",       strlen("isal"),       (int64_t)&str_isal);
    ht_insert_string_int(ht, "isnum",      strlen("isnum"),      (int64_t)&str_isnum);
    ht_insert_string_int(ht, "isspace",    strlen("isspace"),    (int64_t)&str_isspace);
    ht_insert_string_int(ht, "tobool",     strlen("tobool"),     (int64_t)&str_tobool);
    ht_insert_string_int(ht, "tostr",      strlen("tostr"),      (int64_t)&str_tostr);
    ht_insert_string_int(ht, "toupper",    strlen("toupper"),    (int64_t) &str_toupper);
    ht_insert_string_int(ht, "tolower",    strlen("tolower"),    (int64_t) &str_tolower);
    ht_insert_string_int(ht, "startswith", strlen("startswith"), (int64_t)&str_startswith);
    ht_insert_string_int(ht, "endswith",   strlen("endswith"),   (int64_t)&str_endswith);
    ht_insert_string_int(ht, "replace",    strlen("replace"),    (int64_t)&str_replace);
    ht_insert_string_int(ht, "search",     strlen("search"),     (int64_t)&str_search);
    ht_insert_string_int(ht, "split",      strlen("split"),      (int64_t)&str_split);
    ht_insert_string_int(ht, "ltrim",      strlen("ltrim"),      (int64_t)&str_ltrim);
    ht_insert_string_int(ht, "rtrim",      strlen("rtrim"),      (int64_t)&str_rtrim);
    ht_insert_string_int(ht, "trim",       strlen("trim"),       (int64_t)&str_trim);
    ht_insert_string_int(ht, "__get",      strlen("__get"),      (int64_t)&str___get);
    return ht;
}

Hash_t* list_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_string_int(ht, "push", strlen("push"), (int64_t)&list_push);
    ht_insert_string_int(ht, "copy", strlen("copy"), (int64_t)&list_copy);
    ht_insert_string_int(ht, "extend", strlen("extend"), (int64_t)&list_extend);
    ht_insert_string_int(ht, "pop", strlen("pop"), (int64_t)&list_pop);
    ht_insert_string_int(ht, "__get",  strlen("__get"),  (int64_t)&list___get);
    ht_insert_string_int(ht, "__set",  strlen("__set"),  (int64_t)&list___set);
    ht_insert_string_int(ht, "search", strlen("search"), (int64_t)&list_search);
    ht_insert_string_int(ht, "reverse", strlen("reverse"), (int64_t)&list_reverse);
    return ht;
}

Hash_t* table_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_string_int(ht, "keys",   strlen("keys"),   (int64_t) &table_keys);
    ht_insert_string_int(ht, "values", strlen("values"), (int64_t) &table_values);
    ht_insert_string_int(ht, "copy",  strlen("copy"),  (int64_t) &table_clone);
    ht_insert_string_int(ht, "__get",  strlen("__get"),  (int64_t) &table___get);
    ht_insert_string_int(ht, "__set",  strlen("__set"),  (int64_t) &table___set);
    return ht;
}

