#include "str_methods.h"

int str_tobool(VM* vm) {
    Constant a = POP(vm);
    if (((String_t *) a.value)->length == 0) {
        vm->stack[++vm->sp] = FALSE_C;
    } else {
        vm->stack[++vm->sp] = TRUE_C;
    }
    return 0;
}

int str_tostr(VM *vm) {
    return 0;
}

int str_upcase(VM* vm) {
    Constant a = PEEK(vm);
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

int str_downcase(VM* vm) {
    Constant a = PEEK(vm);
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

int str_isalnum(VM* vm) {
    Constant a = POP(vm);
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

int str_isal(VM* vm) {
    Constant a = POP(vm);
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

int str_isnum(VM* vm) {
    Constant a = POP(vm);
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

int str_isspace(VM* vm) {
    Constant a = POP(vm);
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

int str_startswith(VM* vm) {
    Constant haystack = POP(vm);
    Constant needle = POP(vm);
    if (needle.type != STR8) {
        printf("Error: str.startswith(...) expected type %x as first argument, got type %x\n", STR8, needle.type);
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

int str_endswith(VM* vm) {
    Constant haystack = POP(vm);
    Constant needle = POP(vm);
    if (needle.type != STR8) {
        printf("Error: str.startswith(...) expected type %x as first argument, got type %x\n", STR8, needle.type);
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

int str_search(VM* vm) {
    Constant haystack = POP(vm);
    Constant needle = POP(vm);
    if (needle.type != STR8) {
        printf("Error: str.search(...) expected type %x as first argument, got type %x\n", STR8, needle.type);
        return -1;
    }
    if ((((String_t*)haystack.value)->length < ((String_t*)needle.value)->length)) {
        vm->stack[++vm->sp] = (Constant) {INT64, -1};
        return 0;
    }
    vm->stack[++vm->sp] = (Constant) {INT64, string8_search((String_t*)haystack.value, (String_t*)needle.value)};
    return 0;
}

int str_split(VM* vm) {
    Constant haystack = POP(vm);
    Constant needle = POP(vm);
    if (needle.type != STR8) {
        printf("Error: str.split(...) expected type %x as first argument, got type %x\n", STR8, needle.type);
        return -1;
    } else if (((String_t*)needle.value)->length == 0) {
        printf("Error: str.split(...) requires type %x of length > 0 as second argument\n", STR8);
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