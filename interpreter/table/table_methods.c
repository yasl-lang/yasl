#include "table_methods.h"

#include <stdio.h>

#include "yasl_state.h"
#include "VM.h"
#include "YASL_Object.h"

int table___get(struct YASL_State *S) {
    struct YASL_Object key = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_TABLE, "table.__get");
    struct RC_Table* ht = YASL_GETTBL(PEEK(S->vm));
    struct YASL_Object *result = rcht_search(ht, key);
    if (result == NULL) {
        S->vm->sp++;  // TODO: fix this
        //vm_push(S->vm, key);
        return -1;
    }
    else {
        vm_pop(S->vm);
        vm_push(S->vm, *result);
    }
    return 0;
}

int table___set(struct YASL_State *S) {
    struct YASL_Object val = vm_pop(S->vm);
    struct YASL_Object key = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_TABLE, "table.__set");
    struct RC_Table* ht = YASL_GETTBL(vm_pop(S->vm));

    if (YASL_ISLIST(key) || YASL_ISTBL(key) || YASL_ISUSERDATA(key)) {
        printf("Error: unable to use mutable object of type %x as key.\n", key.type);
        return -1;
    }
    rcht_insert(ht, key, val);
    vm_push(S->vm, val);
    return 0;
}

int table_keys(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_TABLE, "table.keys");
    struct RC_Table *ht = YASL_GETTBL(vm_pop(S->vm));
    struct RC_List* ls = ls_new();
    Item_t* item;
    for (size_t i = 0; i < ht->table->size; i++) {
        item = ht->table->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->key));
        }
    }
    vm_push(S->vm, YASL_LIST(ls));
    return 0;
}

int table_values(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_TABLE, "table.values");
    struct RC_Table *ht = YASL_GETTBL(vm_pop(S->vm));
    struct RC_List* ls = ls_new();
    Item_t* item;
    for (size_t i = 0; i < ht->table->size; i++) {
        item = ht->table->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->value));
        }
    }
    vm_push(S->vm, YASL_LIST(ls));
    return 0;
}

int table_clone(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_TABLE, "table.clone");
    struct RC_Table* ht = YASL_GETTBL(vm_pop(S->vm));
    struct RC_Table* new_ht = rcht_new_sized(ht->table->base_size);
    for (size_t i = 0; i < ht->table->size; i++) {
        Item_t* item = ht->table->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            inc_ref(item->key);
            inc_ref(item->value);
            rcht_insert(new_ht, *item->key, *item->value);
        }
    }

    vm_push(S->vm, YASL_TBL(new_ht));
    return 0;
}
