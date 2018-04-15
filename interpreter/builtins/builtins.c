#include <inttypes.h>
#include <string.h>

#include "builtins.h"

int yasl_print(VM* vm) {
    Constant v = vm->stack[vm->sp--];    // pop value from top of the stack ...
    return print(v);
}

int yasl_input(VM* vm) {
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
    vm->stack[++vm->sp].value = (int64_t)new_sized_string8_from_mem(len, str);
    vm->stack[vm->sp].type = STR8;
    return 0;
}

/* int yasl_typeof(VM *vm) {
    return 0;
} */

/* TODO: implement for all types
 * "tofloat64":    0x0A,
 * "toint64":      0x0B,
 * "tobool":       0x0C,
 * "tostr8":       0x0D,
 * "tolist":       0x0E,
 * "tomap":        0x0F,
*/

int yasl_toint64(VM* vm) {
    Constant a = POP(vm);
    if (a.type == FLOAT64) {
        double d;
        memcpy(&d, &a.value, sizeof(d));
        //printf("d = %f\n", d);
        vm->stack[++vm->sp].type = INT64;
        PEEK(vm).value = (int64_t)d;
        return 0;
    }
    printf("toint64(...) is not implemented (yet) for arguments of type %x\n", a.type);
    return -1;
}

int yasl_insert(VM* vm) {
    Constant val = POP(vm);
    Constant key = POP(vm);
    Constant ht  = POP(vm);
    if (ht.type != HASH) {
        printf("Error: insert(...) expected type %x as first argument, got type %x\n", HASH, ht.type);
        return -1;
    } else if (key.type == LIST || key.type == HASH) {
        printf("Error: unable to use mutable object of type %x as key.\n", key.type);
        return -1;
    }
    ht_insert((Hash_t*)ht.value, key, val);
    PUSH(vm, UNDEF_C);
    return 0;
}

int yasl_find(VM* vm) {
    Constant key = POP(vm);
    Constant ht  = POP(vm);
    if (ht.type == HASH) {
        PUSH(vm, *ht_search((Hash_t*)ht.value, key));
        return 0;
    } else if (ht.type == LIST) {
        if (key.type != INT64) {
            printf("Error: find(...) expected type %x as key, got type %x\n", INT64, key.type);
            return -1;
        }
        PUSH(vm, ls_search((List_t*)ht.value, key.value));
        return 0;
    }
    printf("Error: find(...) expected type %x or %x as first argument, got type %x\n", HASH, LIST, ht.type);
    return -1;

}

int yasl_append(VM* vm) {
    Constant ls  = POP(vm);
    Constant val = POP(vm);
    if (ls.type != LIST) {
        printf("Error: expected type %x as first argument, got type %x\n", LIST, ls.type);
        return -1;
    }
    ls_append((List_t*)ls.value, val);
    PUSH(vm, UNDEF_C);
    return 0;
}

int yasl_open(VM* vm) {     //TODO: fix bug relating to file pointer
    Constant mode_str = POP(vm);
    Constant str = POP(vm);
    if (mode_str.type != STR8) {
        printf("Error: open(...) expected type %x as second argument, got type %x\n", STR8, str.type);
        return -1;
    }
    if (str.type != STR8) {
        printf("Error: open(...) expected type %x as first argument, got type %x\n", STR8, str.type);
        return -1;
    }
    char *buffer = malloc(((String_t*)str.value)->length + 1);
    memcpy(buffer, ((String_t*)str.value)->str, ((String_t*)str.value)->length);
    buffer[((String_t*)str.value)->length] = '\0';
    char *mode = malloc(((String_t*)mode_str.value)->length + 1);
    memcpy(mode, ((String_t*)mode_str.value)->str, ((String_t*)mode_str.value)->length);
    mode[((String_t*)mode_str.value)->length] = '\0';

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
    vm->stack[++vm->sp].value = (int64_t)f;
    vm->stack[vm->sp].type = FILEH;
    f = NULL;
    return 0;
}


int yasl_popen(VM* vm) {     //TODO: fix bug relating to file pointer
    Constant mode_str = POP(vm);
    Constant str = POP(vm);
    if (mode_str.type != STR8) {
        printf("Error: popen(...) expected type %x as second argument, got type %x\n", STR8, str.type);
        return -1;
    }
    if (str.type != STR8) {
        printf("Error: popen(...) expected type %x as first argument, got type %x\n", STR8, str.type);
        return -1;
    }
    char *buffer = malloc(((String_t*)str.value)->length + 1);
    memcpy(buffer, ((String_t*)str.value)->str, ((String_t*)str.value)->length);
    buffer[((String_t*)str.value)->length] = '\0';
    char *mode = malloc(((String_t*)mode_str.value)->length + 1);
    memcpy(mode, ((String_t*)mode_str.value)->str, ((String_t*)mode_str.value)->length);
    mode[((String_t*)mode_str.value)->length] = '\0';

    FILE *f;  // r, w, a, r+, w+, a+
    if (!strcmp(mode, "r")) {
        f = popen(buffer, "r");
    } else if (!strcmp(mode, "w")) {
        f = popen(buffer, "w");
    } else {
        printf("Error: invalid second argument: %s\n", mode);
        return -1;
    }
    vm->stack[++vm->sp].value = (int64_t)f;
    vm->stack[vm->sp].type = FILEH;
    f = NULL;
    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                   *
 *                                                                                                                   *
 *                                                 VTABLES                                                           *
 *                                                                                                                   *
 *                                                                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

VTable_t* float64_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x0B, (int64_t)&yasl_toint64);
    return vt;
}

VTable_t* int64_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x0A, (int64_t)&int64_tofloat64);
    return vt;
}

VTable_t* str8_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x10, (int64_t)&str_upcase);
    vt_insert(vt, 0x11, (int64_t)&str_downcase);
    vt_insert(vt, 0x12, (int64_t)&str_isalnum);
    vt_insert(vt, 0x13, (int64_t)&str_isal);
    vt_insert(vt, 0x14, (int64_t)&str_isnum);
    vt_insert(vt, 0x15, (int64_t)&str_isspace);
    vt_insert(vt, 0x16, (int64_t)&str_startswith);
    vt_insert(vt, 0x17, (int64_t)&str_endswith);
    vt_insert(vt, 0x18, (int64_t)&str_search);
    vt_insert(vt, 0x19, (int64_t)&str_split);
    vt_insert(vt, 0x0C, (int64_t)&str_tobool);
    return vt;
}

VTable_t* list_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x20, (int64_t)&yasl_append);
    return vt;
}

VTable_t* hash_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x30, (int64_t)&hash_keys);
    vt_insert(vt, 0x31, (int64_t)&hash_values);
    return vt;
}

VTable_t* file_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x40, (int64_t)&file_close);
    vt_insert(vt, 0x41, (int64_t)&file_pclose);
    vt_insert(vt, 0x42, (int64_t)&file_read);
    vt_insert(vt, 0x43, (int64_t)&file_write);
    vt_insert(vt, 0x44, (int64_t)&file_readline);
    return vt;
}
