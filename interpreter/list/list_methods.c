#include <interpreter/YASL_Object/YASL_Object.h>
#include "list_methods.h"

#include <stdio.h>
#include "VM.h"
#include "YASL_Object.h"
#include "list.h"

int list___get(struct VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.__get");
    List_t *ls = vm_pop(vm).value.lval;
    struct YASL_Object index = PEEK(vm);
    if (!yasl_type_equals(index.type, Y_INT64)) {
        // printf("TypeError: cannot index list with non-integer\n");
        return -1;
    } else if (index.value.ival < -ls->count || index.value.ival >= ls->count) {
        printf("IndexError\n");
        return -1;
    } else {
        if (index.value.ival >= 0) {
            vm_pop(vm);
            vm_push(vm, &ls->items[index.value.ival]);
        }
        else {
            vm_pop(vm);
            vm_push(vm, &ls->items[index.value.ival + ls->count]);
        }
    }
    return 0;
}

int list___set(struct VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.__set");
    List_t *ls = vm_pop(vm).value.lval;
    struct YASL_Object value = vm_pop(vm);
    struct YASL_Object index = vm_pop(vm);
    if (index.type != Y_INT64) {
        printf("TypeError: cannot index list with non-integer\n");
        return -1;
        vm_push(vm, YASL_Undef());
    } else if (index.value.ival < -ls->count || index.value.ival >= ls->count) {
        printf("IndexError\n");
        return -1;
        vm_push(vm, YASL_Undef());
    } else {
        if (index.value.ival >= 0) ls_insert(ls, index.value.ival, value); //  ls->items[index.value.ival] = value;
        else ls_insert(ls, index.value.ival + ls->count, value); //ls->items[index.value.ival + ls->count] = value;
        vm_push(vm, &value);
    }
    return 0;
}


int list_push(struct VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.push");
    struct YASL_Object ls  = vm_pop(vm);
    struct YASL_Object val = vm_pop(vm);
    ls_append(ls.value.lval, val);
    vm_push(vm, &ls);
    return 0;
}

int list_copy(struct VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.copy");
    struct YASL_Object ls = POP(vm);
    List_t *new_ls = ls_new_sized(ls.value.lval->size);
    new_ls->count = ls.value.lval->count;
    memcpy(new_ls->items, ls.value.lval->items, new_ls->count*sizeof(struct YASL_Object));

    struct YASL_Object* yasl_list = malloc(sizeof(struct YASL_Object));
    yasl_list->type = Y_LIST;
    yasl_list->value.lval = new_ls;

    PUSH(vm, *yasl_list);
    return 0;
}

int list_extend(struct VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.extend");
    struct YASL_Object ls  = POP(vm);
    ASSERT_TYPE(vm, Y_LIST, "list.extend");
    struct YASL_Object extend_ls = POP(vm);

    struct List_s *exls = extend_ls.value.lval;
    for(unsigned int i = 0; i < exls->count; i++) {
        ls_append(ls.value.lval, exls->items[i]);
    }
    vm_push(vm, YASL_Undef());
    return 0;
}

int list_pop(struct VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.pop");
    struct YASL_Object ls  = vm_pop(vm);
    if (ls.value.lval->count == 0) {
        puts("cannot pop from empty list.");
        exit(EXIT_FAILURE);
    }
    vm_push(vm, &ls.value.lval->items[--ls.value.lval->count]);
    return 0;
}

int list_search(struct VM* vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.search");
    struct YASL_Object haystack = vm_pop(vm);
    struct YASL_Object needle = vm_pop(vm);
    struct YASL_Object *index = YASL_Undef();
    int i;
    for (i = 0; i < haystack.value.lval->count; i++) {
        if (!isfalsey(isequal(haystack.value.lval->items[i], needle)))
            index = YASL_Integer(i);
    }
    vm_push(vm, index);
    return 0;
}

int list_reverse(struct VM *vm) {
    ASSERT_TYPE(vm, Y_LIST, "list.reverse");
    List_t *ls = vm_pop(vm).value.lval;
    ls_reverse(ls);
    vm_push(vm, YASL_Undef());
    return 0;
}
