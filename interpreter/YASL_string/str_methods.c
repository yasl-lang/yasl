#include <interpreter/YASL_Object/YASL_Object.h>
#include "str_methods.h"
#include "YASL_string.h"

int str___get(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.__get");
    String_t *str = POP(vm).value.sval;
    YASL_Object index = POP(vm);
    if (index.type != Y_INT64) {
        return -1;
        vm_push(vm, UNDEF_C);
    } else if (index.value.ival < -yasl_string_len(str) || index.value.ival >= yasl_string_len(str)) {
        return -1;
        vm_push(vm, UNDEF_C);
    } else {
        if (index.value.ival >= 0) vm_push(vm, ((YASL_Object){Y_STR, (int64_t) str_new_sized_from_mem(index.value.ival, index.value.ival + 1, str->str)}));
        else vm_push(vm, ((YASL_Object){Y_STR, (int64_t) str_new_sized_from_mem(index.value.ival + yasl_string_len(str), index.value.ival + yasl_string_len(str) + 1, str->str)}));
    }
    return 0;
}

int str_tobool(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.tobool");
    YASL_Object a = POP(vm);
    if (yasl_string_len(a.value.sval) == 0) {
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
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    unsigned char curr;
    unsigned char *ptr = malloc(length);

    while (i < length) {
        curr = (a.value.sval)->str[i + a.value.sval->start];
        if (0x61 <= curr && curr < 0x7B) {
            ptr[i++] = curr & ~0x20;
        } else {
            ptr[i++] = curr;
        }
    }

    vm->stack[vm->sp].value.sval = str_new_sized(length, ptr);
    return 0;
}

int str_tolower(VM *vm) {
    ASSERT_TYPE(vm, Y_STR, "str.tolower");
    YASL_Object a = PEEK(vm);
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    unsigned char curr;
    unsigned char *ptr = malloc(length);

    while (i < length) {
        curr = (a.value.sval)->str[i + a.value.sval->start];
        if (0x41 <= curr && curr < 0x5B) {
            ptr[i++] = curr | 0x20;
        } else {
            ptr[i++] = curr;
        }
    }
    vm->stack[vm->sp].value.sval = str_new_sized(length, ptr);
    return 0;
}

int str_isalnum(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.isalnum");
    YASL_Object a = POP(vm);
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++ + a.value.sval->start];
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
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = ((a.value.sval)->str[i++ + a.value.sval->start]);
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
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++ + a.value.sval->start];
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
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++ + a.value.sval->start];
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
    if ((yasl_string_len(haystack.value.sval) < yasl_string_len(needle.value.sval))) {
        vm->stack[++vm->sp] = FALSE_C;
        return 0;
    }
    int64_t i = 0;
    while (i < yasl_string_len(needle.value.sval)) {
        if ((haystack.value.sval)->str[i + haystack.value.sval->start] != (needle.value.sval)->str[i + needle.value.sval->start]) {
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
    if ((yasl_string_len(haystack.value.sval) < yasl_string_len(needle.value.sval))) {
        vm->stack[++vm->sp] = FALSE_C;
        return 0;
    }
    int64_t i = 0;
    while (i < yasl_string_len(needle.value.sval)) {
        if ((haystack.value.sval)->str[i + haystack.value.sval->start + yasl_string_len(haystack.value.sval) - yasl_string_len(needle.value.sval)]
                != (needle.value.sval)->str[i + needle.value.sval->start]) {
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
    if ((yasl_string_len(haystack.value.sval) < yasl_string_len(needle.value.sval))) {
        vm->stack[++vm->sp] = (YASL_Object) {Y_UNDEF, 0};
        return 0;
    }
    int64_t index = str_find_index(haystack.value.sval, needle.value.sval);
    if (index != -1) vm->stack[++vm->sp] = (YASL_Object) {Y_INT64, index };
    else vm->stack[++vm->sp] = (YASL_Object) {Y_UNDEF, 0};
    return 0;
}

// TODO: fix all of these

int str_split(VM* vm) {
    ASSERT_TYPE(vm, Y_STR, "str.split");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    if (needle.type != Y_STR) {
        printf("Error: str.split(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    } else if (yasl_string_len(needle.value.sval) == 0) {
        printf("Error: str.split(...) requires type %x of length > 0 as second argument\n", Y_STR);
        return -1;
    }
    int64_t end=0, start=0;
    List_t* result = ls_new();
    while (end + yasl_string_len(needle.value.sval) <= yasl_string_len(haystack.value.sval)) {
        if (!memcmp(haystack.value.sval->str + haystack.value.sval->start + end,
                    needle.value.sval->str + needle.value.sval->start,
                    yasl_string_len(needle.value.sval))) {
            ls_append(result, (YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(start + haystack.value.sval->start, end + haystack.value.sval->start, haystack.value.sval->str)});
            end += yasl_string_len(needle.value.sval);
            start = end;
        } else {
            end++;
        }
    }
    ls_append(result, (YASL_Object)
            {Y_STR, (int64_t) str_new_sized_from_mem(start + haystack.value.sval->start, end + haystack.value.sval->start, haystack.value.sval->str)});
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
    while(yasl_string_len(haystack.value.sval) - start >= yasl_string_len(needle.value.sval) &&
          !memcmp(haystack.value.sval->str + start,
                  needle.value.sval->str,
                  yasl_string_len(needle.value.sval))) {
        start += yasl_string_len(needle.value.sval);
    }

    vm_push(vm, ((YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(start, yasl_string_len(haystack.value.sval),
                                                                   haystack.value.sval->str)}));

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
    int64_t end = yasl_string_len(haystack.value.sval);
    while(end >= yasl_string_len(needle.value.sval) &&
          !memcmp(haystack.value.sval->str + end - yasl_string_len(needle.value.sval),
                  needle.value.sval->str,
                  yasl_string_len(needle.value.sval))) {
        end -= yasl_string_len(needle.value.sval);
    }

    vm_push(vm, ((YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(haystack.value.sval->start, end, haystack.value.sval->str)}));

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
    while(yasl_string_len(haystack.value.sval) - start >= yasl_string_len(needle.value.sval) &&
          !memcmp(haystack.value.sval->str + start,
                  needle.value.sval->str,
                  yasl_string_len(needle.value.sval))) {
        start += yasl_string_len(needle.value.sval);
    }

    int64_t end = yasl_string_len(haystack.value.sval);
    while(end >= yasl_string_len(needle.value.sval) &&
          !memcmp(haystack.value.sval->str + end - yasl_string_len(needle.value.sval),
                  needle.value.sval->str,
                  yasl_string_len(needle.value.sval))) {
        end -= yasl_string_len(needle.value.sval);
    }

    vm_push(vm, ((YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(start, end, haystack.value.sval->str)}));

    return 0;
}