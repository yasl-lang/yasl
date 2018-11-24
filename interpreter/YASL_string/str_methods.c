#include <interpreter/YASL_Object/YASL_Object.h>
#include <bytebuffer/bytebuffer.h>
#include "str_methods.h"
#include "YASL_string.h"
#include <ctype.h>
#include "yasl_state.h"

int str___get(struct YASL_State *S) {
    struct YASL_Object index = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.__get");
    String_t *str = POP(S->vm).value.sval;
    if (index.type != Y_INT64) {
        return -1;
        vm_push(S->vm, YASL_UNDEF());
    } else if (index.value.ival < -yasl_string_len(str) || index.value.ival >= yasl_string_len(str)) {
        return -1;
        vm_push(S->vm, YASL_UNDEF());
    } else {
        if (index.value.ival >= 0)
            vm_push(S->vm, YASL_STR(str_new_sized_from_mem(str->start + index.value.ival, str->start + index.value.ival + 1, str->str)));
        else vm_push(S->vm, YASL_STR(str_new_sized_from_mem(str->start + index.value.ival + yasl_string_len(str), str->start + index.value.ival + yasl_string_len(str) + 1, str->str)));
    }
    return 0;
}

int isvaliddouble(const char *str) {
	long len = strlen(str);
	int hasdot = 0;
	for (int i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i]) && str[i] != '.' || hasdot && str[i] == '.') {
			return 0;
		}
		if (str[i] == '.') hasdot = 1;
	}
	return hasdot && isdigit(str[len-1]) && isdigit(str[0]);
}

double parsedouble(const char *str) {
	if (!strcmp(str, "inf") || !strcmp(str, "+inf")) return 1.0 / 0.0;
	else if (!strcmp(str, "-inf")) return -1.0 / 0.0;
	else if (str[0] == '-' && isvaliddouble(str+1))
		return -strtod(str+1, NULL);
	else if (str[0] == '+' && isvaliddouble(str+1))
		return +strtod(str+1, NULL);
	else if (isvaliddouble(str))	return strtod(str, NULL);
	return 0.0 / 0.0;
}

int str_tofloat64(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.tofloat64");
    String_t *str = POP(S->vm).value.sval;
    char *buffer = malloc(yasl_string_len(str) + 1);
    memcpy(buffer, str->str + str->start, yasl_string_len(str));
    buffer[yasl_string_len(str)] = '\0';
    vm_push(S->vm, YASL_FLOAT(parsedouble(buffer)));
    free(buffer);
    return 0;
}

int str_tobool(struct YASL_State* S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.tobool");
    struct YASL_Object a = POP(S->vm);
    if (yasl_string_len(a.value.sval) == 0) {
        S->vm->stack[++S->vm->sp] = FALSE_C;
    } else {
        S->vm->stack[++S->vm->sp] = TRUE_C;
    }
    return 0;
}

int str_tostr(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.tostr");
    return 0;
}

int str_toupper(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.toupper");
    struct YASL_Object a = POP(S->vm);
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

    vm_push(S->vm, YASL_STR(str_new_sized(length, ptr)));
    return 0;
}

int str_tolower(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.tolower");
    struct YASL_Object a = POP(S->vm);
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
    vm_push(S->vm, YASL_STR(str_new_sized(length, ptr)));
    return 0;
}

int str_isalnum(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.isalnum");
    struct YASL_Object a = POP(S->vm);
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++ + a.value.sval->start];
        if (curr < 0x30 || (0x3A <= curr && curr < 0x41) || (0x5B <= curr && curr < 0x61) || (0x7B <= curr)) {
            S->vm->stack[++S->vm->sp] = FALSE_C;
            return 0;
        }
    }
    S->vm->stack[++S->vm->sp] = TRUE_C;
    return 0;
}

int str_isal(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.isal");
    struct YASL_Object a = POP(S->vm);
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = ((a.value.sval)->str[i++ + a.value.sval->start]);
        if (curr < 0x41 || (0x5B <= curr && curr < 0x61) || (0x7B <= curr)) {
            S->vm->stack[++S->vm->sp] = FALSE_C;
            return 0;
        }
    }
    S->vm->stack[++S->vm->sp] = TRUE_C;
    return 0;
}

int str_isnum(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.isnum");
    struct YASL_Object a = POP(S->vm);
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++ + a.value.sval->start];
        if (curr < 0x30 || 0x3A <= curr) {
            S->vm->stack[++S->vm->sp] = FALSE_C;
            return 0;
        }
    }
    S->vm->stack[++S->vm->sp] = TRUE_C;
    return 0;

}

int str_isspace(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.isspace");
    struct YASL_Object a = POP(S->vm);
    int64_t length = yasl_string_len(a.value.sval);
    int64_t i = 0;
    char curr;
    while (i < length) {
        curr = (a.value.sval)->str[i++ + a.value.sval->start];
        if (curr <= 0x08 || (0x0D < curr && curr != 0x20 && curr != 0x85 && curr != 0xA0)) {
            S->vm->stack[++S->vm->sp] = FALSE_C;
            return 0;
        }
    }
    S->vm->stack[++S->vm->sp] = TRUE_C;
    return 0;
}

int str_startswith(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.startswith");
    struct YASL_Object needle = POP(S->vm);
    struct YASL_Object haystack = POP(S->vm);
    if (needle.type != Y_STR) {
        printf("Error: str.startswith(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    if ((yasl_string_len(haystack.value.sval) < yasl_string_len(needle.value.sval))) {
        S->vm->stack[++S->vm->sp] = FALSE_C;
        return 0;
    }
    int64_t i = 0;
    while (i < yasl_string_len(needle.value.sval)) {
        if (haystack.value.sval->str[i + haystack.value.sval->start] != needle.value.sval->str[i + needle.value.sval->start]) {
            S->vm->stack[++S->vm->sp] = FALSE_C;
            return 0;
        }
        i++;
    }
    S->vm->stack[++S->vm->sp] = TRUE_C;
    return 0;
}

int str_endswith(struct YASL_State *S) {
    struct YASL_Object needle = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.endswith");
    struct YASL_Object haystack = POP(S->vm);
    if (needle.type != Y_STR) {
        printf("Error: str.startswith(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    if ((yasl_string_len(haystack.value.sval) < yasl_string_len(needle.value.sval))) {
        S->vm->stack[++S->vm->sp] = FALSE_C;
        return 0;
    }
    int64_t i = 0;
    while (i < yasl_string_len(needle.value.sval)) {
        if ((haystack.value.sval)->str[i + haystack.value.sval->start + yasl_string_len(haystack.value.sval) - yasl_string_len(needle.value.sval)]
                != (needle.value.sval)->str[i + needle.value.sval->start]) {
            S->vm->stack[++S->vm->sp] = FALSE_C;
            return 0;
        }
        i++;
    }
    S->vm->stack[++S->vm->sp] = TRUE_C;
    return 0;
}

int str_replace(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_STR, "str.replace");
    struct YASL_Object replace_str = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.replace");
    struct YASL_Object search_str = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.replace");
    struct YASL_Object str = POP(S->vm);

    unsigned char* str_ptr = str.value.sval->str+str.value.sval->start;
    int64_t str_len = yasl_string_len(str.value.sval);
    unsigned char* search_str_ptr = search_str.value.sval->str+search_str.value.sval->start;
    int64_t search_len = yasl_string_len(search_str.value.sval);
    unsigned char* replace_str_ptr = replace_str.value.sval->str+replace_str.value.sval->start;
    if (search_len < 1) {
        printf("Error: str.replace(...) expected search string with length at least 1\n");
        return -1;
    }

    ByteBuffer *buff = bb_new(yasl_string_len(str.value.sval));
    unsigned int i = 0;
    while (i < str_len) {
        if(search_len <= str_len-i && memcmp(str_ptr+i, search_str_ptr, search_len) == 0) {
            bb_append(buff, replace_str_ptr, yasl_string_len(replace_str.value.sval));
            i += search_len;
        } else {
            bb_add_byte(buff, str_ptr[i++]);
        }
    }

    unsigned char *new_str = malloc(buff->count);
    memcpy(new_str, buff->bytes, buff->count);
    S->vm->stack[++S->vm->sp] = (struct YASL_Object){Y_STR, str_new_sized(buff->count, new_str)};

    bb_del(buff);
    return 0;
}

int str_search(struct YASL_State *S) {
    struct YASL_Object needle = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.search");
    struct YASL_Object haystack = POP(S->vm);
    if (needle.type != Y_STR) {
        printf("Error: str.search(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    int64_t index = str_find_index(haystack.value.sval, needle.value.sval);
    if (index != -1) S->vm->stack[++S->vm->sp] = (struct YASL_Object) {Y_INT64, index };
    else S->vm->stack[++S->vm->sp] = (struct YASL_Object) {Y_UNDEF, 0};
    return 0;
}

// TODO: fix all of these

int str_split(struct YASL_State *S) {
    struct YASL_Object needle = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.split");
    struct YASL_Object haystack = POP(S->vm);
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
            ls_append(result, (struct YASL_Object) {Y_STR, (int64_t) str_new_sized_from_mem(start + haystack.value.sval->start, end + haystack.value.sval->start, haystack.value.sval->str)});
            end += yasl_string_len(needle.value.sval);
            start = end;
        } else {
            end++;
        }
    }
    ls_append(result, (struct YASL_Object)
            {Y_STR, (int64_t) str_new_sized_from_mem(start + haystack.value.sval->start, end + haystack.value.sval->start, haystack.value.sval->str)});
    S->vm->stack[++S->vm->sp] = (struct YASL_Object) {Y_LIST, (int64_t)result};
    return 0;
}

int str_ltrim(struct YASL_State *S) {
    struct YASL_Object needle = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.ltrim");
    struct YASL_Object haystack = POP(S->vm);
    if (needle.type != Y_STR) {
        printf("Error: str.ltrim(...) expected type %x as second argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    int64_t start=0;
    while(yasl_string_len(haystack.value.sval) - start >= yasl_string_len(needle.value.sval) &&
          !memcmp(haystack.value.sval->str + haystack.value.sval->start + start,
                  needle.value.sval->str + needle.value.sval->start,
                  yasl_string_len(needle.value.sval))) {
        start += yasl_string_len(needle.value.sval);
    }

    vm_push(S->vm, YASL_STR(str_new_sized_from_mem(haystack.value.sval->start + start, haystack.value.sval->start + yasl_string_len(haystack.value.sval),
                                                                   haystack.value.sval->str)));

    return 0;
}

int str_rtrim(struct YASL_State *S) {
    struct YASL_Object needle = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.rtrim");
    struct YASL_Object haystack = POP(S->vm);
    if (needle.type != Y_STR) {
        printf("Error: str.rtrim(...) expected type %x as second argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }
    int64_t end = yasl_string_len(haystack.value.sval);
    while(end >= yasl_string_len(needle.value.sval) &&
          !memcmp(haystack.value.sval->str + haystack.value.sval->start + end - yasl_string_len(needle.value.sval),
                  needle.value.sval->str + needle.value.sval->start,
                  yasl_string_len(needle.value.sval))) {
        end -= yasl_string_len(needle.value.sval);
    }

    vm_push(S->vm, YASL_STR(str_new_sized_from_mem(haystack.value.sval->start, haystack.value.sval->start + end, haystack.value.sval->str)));

    return 0;
}

int str_trim(struct YASL_State *S) {
    struct YASL_Object needle = POP(S->vm);
    ASSERT_TYPE(S->vm, Y_STR, "str.trim");
    struct YASL_Object haystack = POP(S->vm);
    if (needle.type != Y_STR) {
        printf("Error: str.trim(...) expected type %x as first argument, got type %x\n", Y_STR, needle.type);
        return -1;
    }

    int64_t start=0;
    while(yasl_string_len(haystack.value.sval) - start >= yasl_string_len(needle.value.sval) &&
          !memcmp(haystack.value.sval->str + haystack.value.sval->start + start,
                  needle.value.sval->str + needle.value.sval->start,
                  yasl_string_len(needle.value.sval))) {
        start += yasl_string_len(needle.value.sval);
    }

    int64_t end = yasl_string_len(haystack.value.sval);
    while(end >= yasl_string_len(needle.value.sval) &&
          !memcmp(haystack.value.sval->str + haystack.value.sval->start + end - yasl_string_len(needle.value.sval),
                  needle.value.sval->str + needle.value.sval->start,
                  yasl_string_len(needle.value.sval))) {
        end -= yasl_string_len(needle.value.sval);
    }

    vm_push(S->vm, YASL_STR(str_new_sized_from_mem(haystack.value.sval->start + start, haystack.value.sval->start + end, haystack.value.sval->str)));

    return 0;
}
