#include <inttypes.h>
#include <string.h>

#include "builtins.h"

int yasl_print(VM* vm) {
    Constant v = vm->stack[vm->sp--];    // pop value from top of the stack ...
    return print(v);
    /*int i;
    switch (v.type) {
        case INT64:
            printf("int64: %" PRId64 "\n", v.value);
            break;
        case FLOAT64:
            printf("float64: %f\n", *((double*)&v.value));
            break;
        case BOOL:
            if (v.value == 0) printf("bool: false\n");
            else printf("bool: true\n");
            break;
        case UNDEF:
            printf("undef: undef\n");
            break;
        case STR8:
            printf("str: ");
            int64_t i;
            for (i = 0; i < ((String_t*)v.value)->length; i++) { // TODO: fix hardcoded 8
                //printf("%.*s\n", ((String_t*)v.value)->length, ((String_t*)v.value)->str);
                printf("%c", ((String_t*)v.value)->str[i]);
            }
            printf("\n");
            break;
        case HASH:
            printf("hash: <%" PRIx64 ">\n", v.value);
            break;;
        case LIST:
            printf("list: <%" PRIx64 ">\n", v.value);
            break;
        case FILEH:
            if ((FILE*)v.value == stdin) {
                puts("file: stdin");
            } else if ((FILE*)v.value == stdout) {
                puts("file: stdout");
            } else if ((FILE*)v.value == stderr) {
                puts("file: stderr");
            } else {
                printf("file: <%" PRIx64 ">\n", v.value);
            }
            break;
        default:
            printf("Error, unknown type : %x\n", v.type);
            return -1;
    }
    return 0; */
}

/*
char *inputString(FILE* fp, size_t size){
//The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size); //size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

int main(void){
    char *m;

    printf("input string : ");
    m = inputString(stdin, 10);
    printf("%s\n", m);

    free(m);
    return 0;
}
*/

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

/* TODO: implement for all types
 * "tofloat64":    0x0A,
 * "toint64":      0x0B,
 * "tobool":       0x0C,
 * "tostr8":       0x0D,
 * "tolist":       0x0E,
 * "tomap":        0x0F,
*/

int yasl_tofloat64(VM* vm) {
    Constant a = POP(vm);
    if (a.type == INT64) {
        double d = (double)a.value;
        //printf("d = %f\n", d);
        vm->stack[++vm->sp].type = FLOAT64;
        memcpy(&vm->stack[vm->sp].value, &d, sizeof(d));
        return 0;
    }
    printf("tofloat64(...) is not implemented (yet) for arguments of type %x\n", a.type);
    return -1;
};

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

int yasl_tobool(VM* vm) {
    Constant a = POP(vm);
    if (a.type == STR8) {
        if (((String_t *) a.value)->length == 0) {
            vm->stack[++vm->sp] = FALSE_C;
        }
        else {
            vm->stack[++vm->sp] = TRUE_C;
        }
        return 0;
    }
    printf("tobool(...) is not implemented (yet) for arguments of type %x\n", a.type);
    return -1;
}

int yasl_tostr8(VM* vm);
int yasl_tolist(VM* vm);
int yasl_tomap(VM* vm);

int yasl_upcase(VM* vm) {
    Constant a = PEEK(vm);
    if (a.type != STR8) {
        printf("Error: upcase(...) expected type %x as first argument, got type %x\n", STR8, a.type);
        return -1;
    }
    int64_t length = ((String_t*)a.value)->length;
    int64_t i = 0;
    String_t* ptr = new_sized_string8(length);
    char curr;
    while (i < length) {
        curr = ((String_t*)a.value)->str[i];
        if (0x61 <= curr && curr < 0x7B) {
            ((String_t*)ptr)->str[i++] = curr & ~0x20;
        } else {
            ((String_t*)ptr)->str[i++] = curr;
        }
    }
    vm->stack[vm->sp].value = (int64_t)ptr;
    return 0;
}

int yasl_downcase(VM* vm) {
    Constant a = PEEK(vm);
    if (a.type != STR8) {
        printf("Error: downcase(...) expected type %x as first argument, got type %x\n", STR8, a.type);
        return -1;
    }
    int64_t length = ((String_t*)a.value)->length;
    int64_t i = 0;
    String_t* ptr = new_sized_string8(length);
    char curr;
    while (i < length) {
        curr = ((String_t*)a.value)->str[i];
        if (0x41 <= curr && curr < 0x5B) {
            ((String_t*)ptr)->str[i++] = curr | 0x20;
        } else {
            ((String_t*)ptr)->str[i++] = curr;
        }
    }
    vm->stack[vm->sp].value = (int64_t)ptr;
    return 0;
}

int yasl_isalnum(VM* vm) {
    Constant a = POP(vm);
    if (a.type != STR8) {
        printf("Error: isalnum(...) expected type %x as first argument, got type %x\n", STR8, a.type);
        return -1;
    }
    int64_t length = ((String_t*)a.value)->length;
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = ((String_t*)a.value)->str[i++];
        if (curr < 0x30 || (0x3A <= curr && curr < 0x41) || (0x5B <= curr && curr < 0x61) || (0x7B <= curr)) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int yasl_isal(VM* vm) {
    Constant a = POP(vm);
    if (a.type != STR8) {
        printf("Error: isal(...) expected type %x as first argument, got type %x\n", STR8, a.type);
        return -1;
    }
    int64_t length = ((String_t*)a.value)->length;
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = ((String_t*)a.value)->str[i++];
        if (curr < 0x41 || (0x5B <= curr && curr < 0x61) || (0x7B <= curr)) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int yasl_isnum(VM* vm) {
    Constant a = POP(vm);
    if (a.type != STR8) {
        printf("Error: isnum(...) expected type %x as first argument, got type %x\n", STR8, a.type);
        return -1;
    }
    int64_t length = ((String_t*)a.value)->length;
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = ((String_t*)a.value)->str[i++];
        if (curr < 0x30 || 0x3A <= curr) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;

}

int yasl_isspace(VM* vm) {
    Constant a = POP(vm);
    if (a.type != STR8) {
        printf("Error: isspace(...) expected type %x as first argument, got type %x\n", STR8, a.type);
        return -1;
    }
    int64_t length = ((String_t*)a.value)->length;
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = ((String_t*)a.value)->str[i++];
        if (curr <= 0x08 || (0x0D < curr && curr != 0x20 && curr != 0x85 && curr != 0xA0)) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int yasl_startswith(VM* vm) {
    Constant haystack = POP(vm);
    Constant needle = POP(vm);
    if (haystack.type != STR8) {
        printf("Error: startswith(...) expected type %x as first argument, got type %x\n", STR8, haystack.type);
        return -1;
    } else if (needle.type != STR8) {
        printf("Error: startswith(...) expected type %x as second argument, got type %x\n", STR8, needle.type);
        return -1;
    }
    if ((((String_t*)haystack.value)->length < ((String_t*)needle.value)->length)) {
        vm->stack[++vm->sp] = FALSE_C;
        return 0;
    }
    int64_t i = 0;
    while (i < ((String_t*)needle.value)->length) {
        if (((String_t*)haystack.value)->str[i] != ((String_t*)needle.value)->str[i]) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
        i++;
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int yasl_endswith(VM* vm) {
    Constant haystack = POP(vm);
    Constant needle = POP(vm);
    if (haystack.type != STR8) {
        printf("Error: startswith(...) expected type %x as first argument, got type %x\n", STR8, haystack.type);
        return -1;
    } else if (needle.type != STR8) {
        printf("Error: startswith(...) expected type %x as second argument, got type %x\n", STR8, needle.type);
        return -1;
    }
    if ((((String_t*)haystack.value)->length < ((String_t*)needle.value)->length)) {
        vm->stack[++vm->sp] = FALSE_C;
        return 0;
    }
    int64_t i = 0;
    while (i < ((String_t*)needle.value)->length) {
        if (((String_t*)haystack.value)->str[i + ((String_t*)haystack.value)->length - ((String_t*)needle.value)->length]
                != ((String_t*)needle.value)->str[i]) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
        i++;
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int yasl_search(VM* vm) {
    Constant haystack = POP(vm);
    Constant needle = POP(vm);
    if (haystack.type != STR8) {
        printf("Error: search(...) expected type %x as first argument, got type %x\n", STR8, haystack.type);
        return -1;
    } else if (needle.type != STR8) {
        printf("Error: search(...) expected type %x as second argument, got type %x\n", STR8, needle.type);
        return -1;
    }
    if ((((String_t*)haystack.value)->length < ((String_t*)needle.value)->length)) {
        vm->stack[++vm->sp] = (Constant) {INT64, -1};
        return 0;
    }
    //int64_t i = 0;
    /*while (i < ((String_t*)haystack.value)->length) {
        if (!memcmp(((String_t*)haystack.value)->str+i, ((String_t*)needle.value)->str, ((String_t*)needle.value)->length)) {
            vm->stack[++vm->sp] = (Constant) {INT64, i};
            return 0;
        }
        i++;
    } */
    vm->stack[++vm->sp] = (Constant) {INT64, string8_search((String_t*)haystack.value, (String_t*)needle.value)};
    return 0;
}

int yasl_split(VM* vm) {
    Constant haystack = POP(vm);
    Constant needle = POP(vm);
    if (haystack.type != STR8) {
        printf("Error: split(...) expected type %x as first argument, got type %x\n", STR8, haystack.type);
        return -1;
    } else if (needle.type != STR8) {
        printf("Error: split(...) expected type %x as second argument, got type %x\n", STR8, needle.type);
        return -1;
    } else if (((String_t*)needle.value)->length == 0) {
        printf("Error: split(...) requires type %x of length > 0 as second argument\n", STR8);
        return -1;
    }
    int64_t end=0, start=0;
    List_t* result = new_list();
    while (end + ((String_t*)needle.value)->length <= ((String_t*)haystack.value)->length) {
        if (!memcmp(((String_t*)haystack.value)->str+end,
                    ((String_t*)needle.value)->str,
                    ((String_t*)needle.value)->length)) {
            ls_append(result, (Constant) {STR8, (int64_t)new_sized_string8_from_mem(end - start,
                                                                           ((String_t*)haystack.value)->str+start)});
            end += ((String_t*)needle.value)->length;
            start = end;
        } else {
            end++;
        }
    }
    ls_append(result, (Constant)
            {STR8, (int64_t)new_sized_string8_from_mem(((String_t*)haystack.value)->length - start,
                                     ((String_t*)haystack.value)->str+start)});
    vm->stack[++vm->sp] = (Constant) {LIST, (int64_t)result};
    return 0;
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

int yasl_keys(VM* vm) {
    Constant ht = POP(vm);
    if (ht.type != HASH) {
        printf("Error: keys(...) expected type %x as first argument, got type %x\n", HASH, ht.type);
        return -1;
    }
    List_t* ls = new_list();
    int64_t i;
    Item_t* item;
    for (i = 0; i < ((Hash_t*)ht.value)->size; i++) {
        item = ((Hash_t*)ht.value)->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->key));
        }
    }
    vm->stack[++vm->sp] = (Constant) {LIST, (int64_t)ls};
    return 0;
}

int yasl_values(VM* vm) {
    Constant ht = POP(vm);
    if (ht.type != HASH) {
        printf("Error: values(...) expected type %x as first argument, got type %x\n", HASH, ht.type);
        return -1;
    }
    List_t* ls = new_list();
    int64_t i;
    Item_t* item;
    for (i = 0; i < ((Hash_t*)ht.value)->size; i++) {
        item = ((Hash_t*)ht.value)->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->value));
        }
    }
    vm->stack[++vm->sp] = (Constant) {LIST, (int64_t)ls};
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

int yasl_close(VM* vm) {
    //puts("trying to close file");
    FILE *f = (FILE*)(POP(vm).value);
    if (fclose(f)) {
        puts("Error: unable closing file");
        return -1;
    }
    //puts("closed successfully");
    return 0;
}

int yasl_write(VM* vm) {
    Constant fileh = POP(vm);
    Constant str = POP(vm);
    if (fileh.type != FILEH) {
        printf("Error: write expected type %x as first argument, got type %x\n", FILEH, fileh.type);
        return -1;
    }
    if (str.type != STR8) {
        printf("Error: write expected type %x as first argument, got type %x\n", STR8, str.type);
        return -1;
    }
    char *buffer = malloc(((String_t*)str.value)->length + 1);
    memcpy(buffer, ((String_t*)str.value)->str, ((String_t*)str.value)->length);
    buffer[((String_t*)str.value)->length] = '\0';
    if (fprintf((FILE*)fileh.value, "%s", ((String_t*)str.value)->str) < 0) {
        printf("Error: failed to write to file\n");
        return -1;
    }
    return 0;
}

int yasl_readline(VM* vm) {
    FILE* f = (FILE*)(POP(vm).value);
    int ch;
    size_t len = 0;
    size_t size = 10;
    char *str = realloc(NULL, sizeof(char)*size);

    if (!str) return -1; // ERROR
    while(EOF!=(ch=fgetc(f)) && ch != '\n'){
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
    vt_insert(vt, 0x0A, (int64_t)&yasl_tofloat64);
    return vt;
}

VTable_t* str8_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x10, (int64_t)&yasl_upcase);
    vt_insert(vt, 0x11, (int64_t)&yasl_downcase);
    vt_insert(vt, 0x12, (int64_t)&yasl_isalnum);
    vt_insert(vt, 0x13, (int64_t)&yasl_isal);
    vt_insert(vt, 0x14, (int64_t)&yasl_isnum);
    vt_insert(vt, 0x15, (int64_t)&yasl_isspace);
    vt_insert(vt, 0x16, (int64_t)&yasl_startswith);
    vt_insert(vt, 0x17, (int64_t)&yasl_endswith);
    vt_insert(vt, 0x18, (int64_t)&yasl_search);
    vt_insert(vt, 0x19, (int64_t)&yasl_split);
    vt_insert(vt, 0x0C, (int64_t)&yasl_tobool);
    return vt;
}

VTable_t* list_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x20, (int64_t)&yasl_append);
    return vt;
}

VTable_t* hash_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x30, (int64_t)&yasl_keys);
    vt_insert(vt, 0x31, (int64_t)&yasl_values);
    return vt;
}

VTable_t* file_builtins() {
    VTable_t* vt = new_vtable();
    vt_insert(vt, 0x40, (int64_t)&yasl_close);
    vt_insert(vt, 0x42, (int64_t)&yasl_write);
    vt_insert(vt, 0x43, (int64_t)&yasl_readline);
    return vt;
}
