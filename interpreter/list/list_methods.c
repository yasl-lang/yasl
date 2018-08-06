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
            vm_push(vm, ls->items[index.value.ival]);
        }
        else {
            POP(vm);
            vm_push(vm, ls->items[index.value.ival + ls->count]);
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
        vm_push(vm, UNDEF_C);
    } else if (index.value.ival < -ls->count || index.value.ival >= ls->count) {
        printf("IndexError\n");
        return -1;
        vm_push(vm, UNDEF_C);
    } else {
        if (index.value.ival >= 0) ls->items[index.value.ival] = value;
        else ls->items[index.value.ival + ls->count] = value;
        vm_push(vm, value);
    }
    return 0;
}


int list_append(VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.append");
    YASL_Object ls  = POP(vm);
    YASL_Object val = POP(vm);
    ls_append(ls.value.lval, val);
    vm_push(vm, UNDEF_C);
    return 0;
}

int list_pop(VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.pop");
    YASL_Object ls  = POP(vm);
    if (ls.value.lval->count == 0) {
        puts("cannot pop from empty list.");
        exit(EXIT_FAILURE);
    }
    vm_push(vm, ls.value.lval->items[--ls.value.lval->count]);
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
    vm_push(vm, index);
    return 0;
}

int list_reverse(VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.reverse");
    List_t *ls = POP(vm).value.lval;
    ls_reverse(ls);
    vm_push(vm, YASL_Undef());
    return 0;
}