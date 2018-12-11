#include "list_methods.h"

#include <stdio.h>

#include "VM.h"
#include "YASL_Object.h"
#include "list.h"
#include "yasl_state.h"

int list___get(struct YASL_State *S) {
    struct YASL_Object index = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.__get");
    List_t *ls = YASL_GETLIST(PEEK(S->vm));
    if (!YASL_ISINT(index)) {
        S->vm->sp++;
        return -1;
    } else if (YASL_GETINT(index) < -ls->count || YASL_GETINT(index) >= ls->count) {
        printf("IndexError\n");
        return -1;
    } else {
        if (index.value.ival >= 0) {
            vm_pop(S->vm);
            vm_push(S->vm, ls->items[YASL_GETINT(index)]);
        }
        else {
            vm_pop(S->vm);
            vm_push(S->vm, ls->items[YASL_GETINT(index) + ls->count]);
        }
    }
    return 0;
}

int list___set(struct YASL_State *S) {
    struct YASL_Object value = vm_pop(S->vm);
    struct YASL_Object index = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.__set");
    List_t *ls = YASL_GETLIST(vm_pop(S->vm));
    if (!YASL_ISINT(index)) {
        printf("TypeError: cannot index list with non-integer\n");
        vm_push(S->vm, YASL_UNDEF());
        return -1;
    } else if (YASL_GETINT(index) < -ls->count || YASL_GETINT(index) >= ls->count) {
        printf("%d || %d\n", YASL_GETINT(index) < -ls->count, YASL_GETINT(index) >= ls->count);
        printf("IndexError\n");
        vm_push(S->vm, YASL_UNDEF());
        return -1;
    } else {
        if (YASL_GETINT(index) >= 0) ls->items[YASL_GETINT(index)] = value;
        else ls->items[YASL_GETINT(index) + ls->count] = value;
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
    List_t *ls = YASL_GETLIST(vm_pop(S->vm));
    List_t *new_ls = ls_new_sized(ls->size);
    new_ls->count = ls->count;
    memcpy(new_ls->items, ls->items, new_ls->count*sizeof(struct YASL_Object));

    struct YASL_Object* yasl_list = malloc(sizeof(struct YASL_Object));
    yasl_list->type = Y_LIST;
    yasl_list->value.lval = new_ls;

    vm_push(S->vm, *yasl_list);

    return 0;
}

int list_extend(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.extend");
    List_t *extend_ls = YASL_GETLIST(vm_pop(S->vm));
    ASSERT_TYPE(S->vm, Y_LIST, "list.extend");
    List_t *ls  = YASL_GETLIST(vm_pop(S->vm));

    struct List_s *exls = extend_ls;
    for(unsigned int i = 0; i < exls->count; i++) {
        ls_append(ls, exls->items[i]);
    }
    vm_push(S->vm, YASL_UNDEF());
    return 0;
}

int list_pop(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.pop");
    List_t *ls  = YASL_GETLIST(vm_pop(S->vm));
    if (ls->count == 0) {
        puts("cannot pop from empty list.");
        exit(EXIT_FAILURE);
    }
    vm_push(S->vm, ls->items[--ls->count]);
    return 0;
}

int list_search(struct YASL_State *S) {
    struct YASL_Object needle = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.search");
    List_t *haystack = YASL_GETLIST(vm_pop(S->vm));
    struct YASL_Object index = YASL_UNDEF();
    int i;
    for (i = 0; i < haystack->count; i++) {
        if (!isfalsey(isequal(haystack->items[i], needle)))
            index = YASL_INT(i);
    }
    vm_push(S->vm, index);
    return 0;
}

int list_reverse(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.reverse");
    List_t *ls = YASL_GETLIST(vm_pop(S->vm));
    ls_reverse(ls);
    vm_push(S->vm, YASL_UNDEF());
    return 0;
}
