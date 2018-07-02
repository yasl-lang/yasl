#include <interpreter/YASL_Object/YASL_Object.h>
#include "str_methods.h"
#include "YASL_string.h"

int str___get(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.__get");
    String_t *str = POP(vm).value.sval;
    YASL_Object index = POP(vm);
    if (index.type != Y_INT64) {
        return -1;
        PUSH(vm, UNDEF_C);
    } else if (index.value.ival < -str->length || index.value.ival >= str->length) {
        return -1;
        PUSH(vm, UNDEF_C);
    } else {
        if (index.value.ival >= 0) PUSH(vm, ((YASL_Object){Y_STR, (int64_t) str_new_sized_from_mem(1, str->str +
                                                                                                    index.value.ival)}));
        else PUSH(vm, ((YASL_Object){Y_STR, (int64_t) str_new_sized_from_mem(1, str->str + index.value.ival + str->length)}));
    }
    return 0;
}

int str_tobool(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.tobool");
    YASL_Object a = POP(vm);
    if ((a.value.sval)->length == 0) {
        vm->stack[++vm->sp] = FALSE_C;
    } else {
        vm->stack[++vm->sp] = TRUE_C;
    }
    return 0;
}

int str_tostr(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.tostr");
    return 0;
}

int str_toupper(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.toupper");
    YASL_Object a = PEEK(vm);
    int64_t length = (a.value.sval)->length;
    int64_t i = 0;
    String_t* ptr = str_new_sized(length);
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i];
        if (0x61 <= curr && curr < 0x7B) {
            ((String_t*)ptr)->str[i++] = curr & ~0x20;
        } else {
            ((String_t*)ptr)->str[i++] = curr;
        }
    }
    vm->stack[vm->sp].value.sval = ptr;
    return 0;
}

int str_tolower(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.tolower");
    YASL_Object a = PEEK(vm);
    int64_t length = (a.value.sval)->length;
    int64_t i = 0;
    String_t* ptr = str_new_sized(length);
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i];
        if (0x41 <= curr && curr < 0x5B) {
            (ptr)->str[i++] = curr | 0x20;
        } else {
            ((String_t*)ptr)->str[i++] = curr;
        }
    }
    vm->stack[vm->sp].value.sval = ptr;
    return 0;
}

int str_isalnum(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.isalnum");
    YASL_Object a = POP(vm);
    int64_t length = (a.value.sval)->length;
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++];
        if (curr < 0x30 || (0x3A <= curr && curr < 0x41) || (0x5B <= curr && curr < 0x61) || (0x7B <= curr)) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int str_isal(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.isal");
    YASL_Object a = POP(vm);
    int64_t length = (a.value.sval)->length;
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = ((a.value.sval)->str[i++]);
        if (curr < 0x41 || (0x5B <= curr && curr < 0x61) || (0x7B <= curr)) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int str_isnum(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.isnum");
    YASL_Object a = POP(vm);
    int64_t length = (a.value.sval)->length;
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++];
        if (curr < 0x30 || 0x3A <= curr) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;

}

int str_isspace(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.isspace");
    YASL_Object a = POP(vm);
    int64_t length = (a.value.sval)->length;
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++];
        if (curr <= 0x08 || (0x0D < curr && curr != 0x20 && curr != 0x85 && curr != 0xA0)) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int str_startswith(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.startswith");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    if (needle.type != Y_STR) {
        printf("Error: str.startswith(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    if (((haystack.value.sval)->length < (needle.value.sval)->length)) {
        vm->stack[++vm->sp] = FALSE_C;
        return 0;
    }
    int64_t i = 0;
    while (i < (needle.value.sval)->length) {
        if ((haystack.value.sval)->str[i] != (needle.value.sval)->str[i]) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
        i++;
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int str_endswith(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.endswith");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    if (needle.type != Y_STR) {
        printf("Error: str.startswith(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    if (((haystack.value.sval)->length < (needle.value.sval)->length)) {
        vm->stack[++vm->sp] = FALSE_C;
        return 0;
    }
    int64_t i = 0;
    while (i < (needle.value.sval)->length) {
        if ((haystack.value.sval)->str[i + (haystack.value.sval)->length - (needle.value.sval)->length]
                != (needle.value.sval)->str[i]) {
            vm->stack[++vm->sp] = FALSE_C;
            return 0;
        }
        i++;
    }
    vm->stack[++vm->sp] = TRUE_C;
    return 0;
}

int str_search(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.search");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    if (needle.type != Y_STR) {
        printf("Error: str.search(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    if (((haystack.value.sval)->length < (needle.value.sval)->length)) {
        vm->stack[++vm->sp] = (YASL_Object) {Y_UNDEF, 0};
        return 0;
    }
    int64_t index = str_find_index(haystack.value.sval, needle.value.sval);
    if (index != -1) vm->stack[++vm->sp] = (YASL_Object) {Y_INT64, index };
    else vm->stack[++vm->sp] = (YASL_Object) {Y_UNDEF, 0};
    return 0;
}

int str_split(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.split");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    if (needle.type != Y_STR) {
        printf("Error: str.split(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    } else if ((needle.value.sval)->length == 0) {
        printf("Error: str.split(...) requires type %x of length > 0 as second argument\n", Y_STR);
        return -1;
    }
    int64_t end=0, start=0;
    List_t* result = ls_new();
    while (end + (needle.value.sval)->length <= (haystack.value.sval)->length) {
        if (!memcmp((haystack.value.sval)->str+end,
                    (needle.value.sval)->str,
                    (needle.value.sval)->length)) {
            ls_append(result, (YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(end - start,
                                                                                   (haystack.value.sval)->str + start)});
            end += (needle.value.sval)->length;
            start = end;
        } else {
            end++;
        }
    }
    ls_append(result, (YASL_Object)
            {Y_STR, (int64_t) str_new_sized_from_mem((haystack.value.sval)->length - start,
                                                   (haystack.value.sval)->str + start)});
    vm->stack[++vm->sp] = (YASL_Object) {Y_LIST, (int64_t)result};
    return 0;
}

int str_ltrim(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.ltrim");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    if (needle.type != Y_STR) {
        printf("Error: str.ltrim(...) expected type %x as second argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    int64_t start=0;
    while(haystack.value.sval->length - start >= needle.value.sval->length &&
          !memcmp(haystack.value.sval->str + start,
                  needle.value.sval->str,
                  needle.value.sval->length)) {
        start += needle.value.sval->length;
    }

    PUSH(vm, ((YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(haystack.value.sval->length - start,
                                                                   haystack.value.sval->str + start)}));

    return 0;
}

int str_rtrim(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.rtrim");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    if (needle.type != Y_STR) {
        printf("Error: str.rtrim(...) expected type %x as second argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    int64_t end=haystack.value.sval->length;
    while(end >= needle.value.sval->length &&
          !memcmp(haystack.value.sval->str + end - needle.value.sval->length,
                  needle.value.sval->str,
                  needle.value.sval->length)) {
        end -= needle.value.sval->length;
    }

    PUSH(vm, ((YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(end, haystack.value.sval->str)}));

    return 0;
}

int str_trim(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.trim");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    if (needle.type != Y_STR) {
        printf("Error: str.trim(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }

    int64_t start=0;
    while(haystack.value.sval->length - start >= needle.value.sval->length &&
          !memcmp(haystack.value.sval->str + start,
                  needle.value.sval->str,
                  needle.value.sval->length)) {
        start += needle.value.sval->length;
    }

    int64_t end=haystack.value.sval->length;
    while(end >= needle.value.sval->length &&
          !memcmp(haystack.value.sval->str + end - needle.value.sval->length,
                  needle.value.sval->str,
                  needle.value.sval->length)) {
        end -= needle.value.sval->length;
    }

    PUSH(vm, ((YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(end - start, haystack.value.sval->str + start)}));

    return 0;
}