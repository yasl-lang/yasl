#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "VM.c"
#include "opcode.c"
#include "constant/constant.c"
#include "hashtable/hashtable.c"
#include "list/list.c"
#include "string8/string8.c"

typedef int (*Handler)(VM*);

int yasl_print(VM* vm) {
    Constant v = vm->stack[vm->sp--];    // pop value from top of the stack ...
    int i;
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
        default:
            printf("Error, unknown type: %x\n", v.type);
            return -1;
    }
    return 0;
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
    puts("input(...) not yet implemented.");
    return -1;
    /*
    Constant v = vm->stack[vm->sp];
    if (v.type != STR8) {
        printf("Error: input(...) expected type %x as first argument, got type %x\n", STR8, v.type);
        return -1;
    }
    yasl_print(vm);
    // TODO: get user input
    return 0; */
}

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
    Constant needle = POP(vm);
    Constant haystack = POP(vm);
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
    Constant needle = POP(vm);
    Constant haystack = POP(vm);
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
    // TODO: implement non-naive algorithm for string search.
    Constant needle = POP(vm);
    Constant haystack = POP(vm);
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
    int64_t i = 0;
    while (i < ((String_t*)haystack.value)->length) {
        if (!memcmp(((String_t*)haystack.value)->str+i, ((String_t*)needle.value)->str, ((String_t*)needle.value)->length)) {
            vm->stack[++vm->sp] = (Constant) {INT64, i};
            return 0;
        }
        i++;
    }
    vm->stack[++vm->sp] = (Constant) {INT64, -1};
    return 0;
}

int yasl_split(VM* vm) {
    Constant needle = POP(vm);
    Constant haystack = POP(vm);
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
    int64_t i = 0;
    int64_t j = 0;
    while (i + ((String_t*)needle.value)->length <= ((String_t*)haystack.value)->length) {
        if (((String_t*)haystack.value)->str[i] == ((String_t*)needle.value)->str[j]);
    }
    // " b", "a b c d e"
    puts("split(...) not yet implemented.");
    return -1;
}

/*
    Constant a = POP(vm);
    Constant b = POP(vm);
    if (a.type != STR8) {
        printf("Error: split(...) expected type %x as first argument, got type %x\n", STR8, a.type);
        return -1;
    }
    if (b.type != STR8) {
        printf("Error: split(...) expected type %x as second argument, got type %x\n", STR8, b.type);
        return -1;
    }
    int64_t length = *((int64_t*)a.value);
    int64_t i = sizeof(int64_t);
    char curr;
    while (i < length + sizeof(int64_t)) {
        curr = *((char*)(a.value + i++));
        if (curr <= 0x08 || (0x0D < curr && curr != 0x20 && curr != 0x85 && curr != 0xA0)) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
} */


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
        //Constant v = *ht_search((Hash_t*)ht.value, key);
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
    Constant val = POP(vm);
    Constant ls  = POP(vm);
    if (ls.type != LIST) {
        printf("Error: expected type %x as first argument, got type %x\n", LIST, ls.type);
        return -1;
    }
    ls_append((List_t*)ls.value, val);
    PUSH(vm, UNDEF_C);
    return 0;
}


const Handler builtins[] = {
    yasl_print,    yasl_upcase, yasl_downcase, yasl_isalnum, yasl_isal,   yasl_isnum, yasl_isspace, yasl_startswith,
    yasl_endswith, yasl_search, yasl_insert,   yasl_find,    yasl_append,
};

const Handler stdio_builtins[] = {
    yasl_print,
};

const Handler stdstr_builtins[] = {
    yasl_upcase, yasl_downcase, yasl_isalnum, yasl_isal, yasl_isnum, yasl_isspace, yasl_startswith,
};

const Handler stdobj_builtins[] = {
    yasl_insert, yasl_find, yasl_append,
};