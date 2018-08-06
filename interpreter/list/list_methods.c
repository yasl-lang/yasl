#include <interpreter/YASL_Object/YASL_Object.h>
#include "list_methods.h"

int list___get(VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.__get");
    List_t *ls = POP(vm).value.lval;
    YASL_Object index = PEEK(vm);
    if (index.type != Y_INT64) {
        return -1;
    } else if (index.value.ival < -ls->count || index.value.ival >= ls->count) {
        printf("IndexError\n");
        return -1;
    } else {
        if (index.value.ival >= 0) {
            POP(vm);
            PUSH(vm, ls->items[index.value.ival]);
        }
        else {
            POP(vm);
            PUSH(vm, ls->items[index.value.ival + ls->count]);
        }
    }
    return 0;
}

int list___set(VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.__set");
    List_t *ls = POP(vm).value.lval;
    YASL_Object value = POP(vm);
    YASL_Object index = POP(vm);
    if (index.type != Y_INT64) {
        return -1;
        PUSH(vm, UNDEF_C);
    } else if (index.value.ival < -ls->count || index.value.ival >= ls->count) {
        printf("IndexError\n");
        return -1;
        PUSH(vm, UNDEF_C);
    } else {
        if (index.value.ival >= 0) ls->items[index.value.ival] = value;
        else ls->items[index.value.ival + ls->count] = value;
        PUSH(vm, value);
    }
    return 0;
}


int list_append(VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.append");
    YASL_Object ls  = POP(vm);
    YASL_Object val = POP(vm);
    ls_append(ls.value.lval, val);
    PUSH(vm, UNDEF_C);
    return 0;
}

int list_copy(VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.copy");
    YASL_Object ls = POP(vm);
    List_t *new_ls = ls_new_sized(ls.value.lval->size);
    new_ls->count = ls.value.lval->count;
    memcpy(new_ls->items, ls.value.lval->items, new_ls->count*sizeof(YASL_Object));

    YASL_Object* yasl_list = malloc(sizeof(YASL_Object));
    yasl_list->type = Y_LIST;
    yasl_list->value.lval = new_ls;

    PUSH(vm, *yasl_list);
    return 0;
}

int list_extend(VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.extend");
    YASL_Object ls  = POP(vm);
    ASSERT_TYPE(vm, Y_LIST, "list.extend");
    YASL_Object extend_ls = POP(vm);

    struct List_s *exls = extend_ls.value.lval;
    for(unsigned int i = 0; i < exls->count; i++) {
        ls_append(ls.value.lval, exls->items[i]);
    }
    PUSH(vm, UNDEF_C);
    return 0;
}

int list_pop(VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.pop");
    YASL_Object ls  = POP(vm);
    if (ls.value.lval->count == 0) {
        puts("cannot pop from empty list.");
        exit(EXIT_FAILURE);
    }
    PUSH(vm, ls.value.lval->items[--ls.value.lval->count]);
    return 0;
}

int list_search(VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.search");
    YASL_Object haystack = POP(vm);
    YASL_Object needle = POP(vm);
    YASL_Object index = UNDEF_C;
    int i;
    for (i = 0; i < haystack.value.lval->count; i++) {
        if (!isfalsey(isequal(haystack.value.lval->items[i], needle)))
            index = (YASL_Object) { .type = Y_INT64, .value.ival = i };
    }
    PUSH(vm, index);
    return 0;
}

int list_reverse(VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.reverse");
    List_t *ls = POP(vm).value.lval;
    ls_reverse(ls);
    return 0;
}
