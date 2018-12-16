#include "list_methods.h"

#include <stdio.h>

#include "VM.h"
#include "YASL_Object.h"
#include "list.h"
#include "yasl_state.h"

int list___get(struct YASL_State *S) {
    struct YASL_Object index = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.__get");
    struct RC_List *ls = YASL_GETLIST(PEEK(S->vm));
    if (!YASL_ISINT(index)) {
        S->vm->sp++;
        return -1;
    } else if (YASL_GETINT(index) < -ls->list.count || YASL_GETINT(index) >= ls->list.count) {
        printf("IndexError\n");
        return -1;
    } else {
        if (index.value.ival >= 0) {
            vm_pop(S->vm);
            vm_push(S->vm, ls->list.items[YASL_GETINT(index)]);
        }
        else {
            vm_pop(S->vm);
            vm_push(S->vm, ls->list.items[YASL_GETINT(index) + ls->list.count]);
        }
    }
    return 0;
}

int list___set(struct YASL_State *S) {
    struct YASL_Object value = vm_pop(S->vm);
    struct YASL_Object index = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.__set");
    struct RC_List *ls = YASL_GETLIST(vm_pop(S->vm));
    if (!YASL_ISINT(index)) {
        printf("TypeError: cannot index list with non-integer\n");
        vm_push(S->vm, YASL_UNDEF());
        return -1;
    } else if (YASL_GETINT(index) < -ls->list.count || YASL_GETINT(index) >= ls->list.count) {
        printf("%d || %d\n", YASL_GETINT(index) < -ls->list.count, YASL_GETINT(index) >= ls->list.count);
        printf("IndexError\n");
        vm_push(S->vm, YASL_UNDEF());
        return -1;
    } else {
        if (YASL_GETINT(index) >= 0) ls->list.items[YASL_GETINT(index)] = value;
        else ls->list.items[YASL_GETINT(index) + ls->list.count] = value;
        vm_push(S->vm, value);
    }
    return 0;
}


int list_push(struct YASL_State *S) {
    struct YASL_Object val = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.push");
    ls_append(YASL_GETLIST(PEEK(S->vm)), val);
    return 0;
}

int list_copy(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.copy");
    struct RC_List *ls = YASL_GETLIST(vm_pop(S->vm));
    struct RC_List *new_ls = ls_new_sized(ls->list.size);
    new_ls->list.count = ls->list.count;
    memcpy(new_ls->list.items, ls->list.items, new_ls->list.count*sizeof(struct YASL_Object));

    struct YASL_Object* yasl_list = malloc(sizeof(struct YASL_Object));
    yasl_list->type = Y_LIST;
    yasl_list->value.lval = new_ls;

    vm_push(S->vm, *yasl_list);

    return 0;
}

int list_extend(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.extend");
    struct RC_List *extend_ls = YASL_GETLIST(vm_pop(S->vm));
    ASSERT_TYPE(S->vm, Y_LIST, "list.extend");
    struct RC_List *ls  = YASL_GETLIST(vm_pop(S->vm));

    struct RC_List *exls = extend_ls;
    for(size_t i = 0; i < exls->list.count; i++) {
        ls_append(ls, exls->list.items[i]);
    }
    vm_push(S->vm, YASL_UNDEF());
    return 0;
}

int list_pop(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.pop");
    struct RC_List *ls  = YASL_GETLIST(vm_pop(S->vm));
    if (ls->list.count == 0) {
        puts("cannot pop from empty list.");
        exit(EXIT_FAILURE);
    }
    vm_push(S->vm, ls->list.items[--ls->list.count]);
    return 0;
}

int list_search(struct YASL_State *S) {
    struct YASL_Object needle = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.search");
    struct RC_List *haystack = YASL_GETLIST(vm_pop(S->vm));
    struct YASL_Object index = YASL_UNDEF();
    int i;
    for (i = 0; i < haystack->list.count; i++) {
        if (!isfalsey(isequal(haystack->list.items[i], needle)))
            index = YASL_INT(i);
    }
    vm_push(S->vm, index);
    return 0;
}

int list_reverse(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.reverse");
    struct RC_List *ls = YASL_GETLIST(vm_pop(S->vm));
    ls_reverse(ls);
    vm_push(S->vm, YASL_UNDEF());
    return 0;
}
