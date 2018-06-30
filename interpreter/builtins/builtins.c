#include <inttypes.h>
#include <string.h>
#include <debug.h>

#include "builtins.h"

int yasl_print(VM* vm) {
    YASL_Object v = vm->stack[vm->sp--];    // pop value from top of the stack ...
    if (v.type == LIST) {
        ls_print(v.value.lval);
        printf("\n");
        return 0;
    } else if (v.type == TABLE) {
        ht_print(v.value.mval);
        printf("\n");
        return 0;
    }
    int return_value = print(v);
    printf("\n");
    return return_value;
}

int yasl_input(VM* vm) {
    print(POP(vm));
    printf("\n");
    int ch;
    size_t len = 0;
    size_t size = 10;
    char *str = realloc(NULL, sizeof(char)*size);

    if (!str) return -1; // ERROR
    while(EOF!=(ch=fgetc(stdin)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return -1; // ERROR
        }
    }
    str = realloc(str, sizeof(char)*len);
    vm->stack[++vm->sp].value.sval = str_new_sized_from_mem(len, str);
    vm->stack[vm->sp].type = STR;
    return 0;
}

int yasl_open(VM* vm) {     //TODO: fix bug relating to file pointer
    YASL_Object mode_str = POP(vm);
    YASL_Object str = POP(vm);
    if (mode_str.type != STR) {
        printf("Error: open(...) expected type %s as second argument, got type %s\n", YASL_TYPE_NAMES[STR], YASL_TYPE_NAMES[str.type]);
        return -1;
    }
    if (str.type != STR) {
        printf("Error: open(...) expected type %s as first argument, got type %s\n", YASL_TYPE_NAMES[STR], YASL_TYPE_NAMES[str.type]);
        return -1;
    }
    char *buffer = malloc((str.value.sval)->length + 1);
    memcpy(buffer, (str.value.sval)->str, (str.value.sval)->length);
    buffer[(str.value.sval)->length] = '\0';
    char *mode = malloc((mode_str.value.sval)->length + 1);
    memcpy(mode, (mode_str.value.sval)->str, (mode_str.value.sval)->length);
    mode[(mode_str.value.sval)->length] = '\0';

    FILE *f;  // r, w, a, r+, w+, a+
    if (!strcmp(mode, "r")) {
        f = fopen(buffer, "r");
    } else if (!strcmp(mode, "w")) {
        f = fopen(buffer, "w");
    } else if (!strcmp(mode, "a")) {
        f = fopen(buffer, "a");
    } else if (!strcmp(mode, "r+")) {
        f = fopen(buffer, "r+");
    } else if (!strcmp(mode, "w+")) {
        f = fopen(buffer, "w+");
    } else if (!strcmp(mode, "a+")) {
        f = fopen(buffer, "a+");
    } else if (!strcmp(mode, "rb")) {
        f = fopen(buffer, "rb");
    } else if (!strcmp(mode, "wb")) {
        f = fopen(buffer, "wb");
    } else if (!strcmp(mode, "ab")) {
        f = fopen(buffer, "ab");
    } else {
        printf("Error: invalid second argument: %s\n", mode);
        return -1;
    }
    vm->stack[++vm->sp].value.fval = f;
    vm->stack[vm->sp].type = FILEH;
    YASL_DEBUG_LOG("%s\n", "file opened successfully.");
    return 0;
}


int yasl_popen(VM* vm) {     //TODO: fix bug relating to file pointer
    YASL_Object mode_str = POP(vm);
    YASL_Object str = POP(vm);
    if (mode_str.type != STR) {
        printf("Error: popen(...) expected type %x as second argument, got type %x\n", STR, str.type);
        return -1;
    }
    if (str.type != STR) {
        printf("Error: popen(...) expected type %x as first argument, got type %x\n", STR, str.type);
        return -1;
    }
    char *buffer = malloc((str.value.sval)->length + 1);
    memcpy(buffer, (str.value.sval)->str, (str.value.sval)->length);
    buffer[(str.value.sval)->length] = '\0';
    char *mode = malloc((mode_str.value.sval)->length + 1);
    memcpy(mode, (mode_str.value.sval)->str, (mode_str.value.sval)->length);
    mode[(mode_str.value.sval)->length] = '\0';

    FILE *f;  // r, w, a, r+, w+, a+
    if (!strcmp(mode, "r")) {
        f = popen(buffer, "r");
    } else if (!strcmp(mode, "w")) {
        f = popen(buffer, "w");
    } else {
        printf("Error: invalid second argument: %s\n", mode);
        return -1;
    }
    vm->stack[++vm->sp].value.fval = f;
    vm->stack[vm->sp].type = FILEH;
    YASL_DEBUG_LOG("%s\n", "process opened successfully.");
    return 0;
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
    ht_insert_string_int(ht, "tobool",     strlen("tobool"),     (int64_t)&str_tobool);
    ht_insert_string_int(ht, "tostr",      strlen("tostr"),      (int64_t)&str_tostr);
    ht_insert_string_int(ht, "toupper",    strlen("toupper"),    (int64_t)&str_upcase);
    ht_insert_string_int(ht, "tolower",    strlen("tolower"),    (int64_t)&str_downcase);
    ht_insert_string_int(ht, "isalnum",    strlen("isalnum"),    (int64_t)&str_isalnum);
    ht_insert_string_int(ht, "isal",       strlen("isal"),       (int64_t)&str_isal);
    ht_insert_string_int(ht, "isnum",      strlen("isnum"),      (int64_t)&str_isnum);
    ht_insert_string_int(ht, "isspace",    strlen("isspace"),    (int64_t)&str_isspace);
    ht_insert_string_int(ht, "startswith", strlen("startswith"), (int64_t)&str_startswith);
    ht_insert_string_int(ht, "endswith",   strlen("endswith"),   (int64_t)&str_endswith);
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
    ht_insert_string_int(ht, "append", strlen("append"), (int64_t)&list_append);
    ht_insert_string_int(ht, "__get",  strlen("__get"),  (int64_t)&list___get);
    ht_insert_string_int(ht, "__set",  strlen("__set"),  (int64_t)&list___set);
    ht_insert_string_int(ht, "search", strlen("search"), (int64_t)&list_search);
    return ht;
}

Hash_t* table_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_string_int(ht, "keys",   strlen("keys"),   (int64_t)&map_keys);
    ht_insert_string_int(ht, "values", strlen("values"), (int64_t)&map_values);
    ht_insert_string_int(ht, "clone",  strlen("clone"),  (int64_t)&map_clone);
    ht_insert_string_int(ht, "__get",  strlen("__get"),  (int64_t)&map___get);
    ht_insert_string_int(ht, "__set",  strlen("__set"),  (int64_t)&map___set);
    return ht;
}

Hash_t* file_builtins() {
    Hash_t* ht = ht_new();
    ht_insert_string_int(ht, "close",    strlen("close"),    (int64_t)&file_close);
    ht_insert_string_int(ht, "pclose",   strlen("pclose"),   (int64_t)&file_pclose);
    ht_insert_string_int(ht, "read",     strlen("read"),     (int64_t)&file_read);
    ht_insert_string_int(ht, "write",    strlen("write"),    (int64_t)&file_write);
    ht_insert_string_int(ht, "readline", strlen("readline"), (int64_t)&file_readline);
    return ht;
}

