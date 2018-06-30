#include <interpreter/YASL_Object/YASL_Object.h>
#include "table_methods.h"
#include "hashtable.h"

int table___get(VM *vm) {
    ASSERT_TYPE(vm, TABLE, "table.__get");
    Hash_t* ht = POP(vm).value.mval;
    YASL_Object key = PEEK(vm);
    YASL_Object *result = ht_search(ht, key);
    if (result == NULL) return -1;
    else {
        POP(vm);
        PUSH(vm, *result);
    }
    return 0;
}

int table___set(VM *vm) {
    ASSERT_TYPE(vm, TABLE, "table.__set");
    Hash_t* ht = POP(vm).value.mval;
    YASL_Object val = POP(vm);
    YASL_Object key = POP(vm);
    if (key.type == LIST || key.type == TABLE) {
        printf("Error: unable to use mutable object of type %x as key.\n", key.type);
        return -1;
    }
    ht_insert(ht, key, val);
    PUSH(vm, val);
    return 0;
}

int table_keys(VM *vm) {
    ASSERT_TYPE(vm, TABLE, "table.keys");
    YASL_Object ht = POP(vm);
    List_t* ls = ls_new();
    int64_t i;
    Item_t* item;
    for (i = 0; i < (ht.value.mval)->size; i++) {
        item = (ht.value.mval)->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->key));
        }
    }
    vm->stack[++vm->sp] = (YASL_Object) {LIST, (int64_t)ls};
    return 0;
}

int table_values(VM *vm) {
    ASSERT_TYPE(vm, TABLE, "table.values");
    YASL_Object ht = POP(vm);
    List_t* ls = ls_new();
    int64_t i;
    Item_t* item;
    for (i = 0; i < (ht.value.mval)->size; i++) {
        item = (ht.value.mval)->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->value));
        }
    }
    vm->stack[++vm->sp] = (YASL_Object) {LIST, (int64_t)ls};
    return 0;
}

int table_clone(VM *vm) {
    ASSERT_TYPE(vm, TABLE, "table.clone");
    Hash_t* ht = POP(vm).value.mval;
    Hash_t* new_ht = ht_new_sized(ht->base_size);
    int i;
    for (i = 0; i < ht->size; i++) {
        Item_t* item = ht->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ht_insert(new_ht, *item->key, *item->value);
        }
    }
    vm->stack[++vm->sp] = (YASL_Object) {TABLE, (int64_t)new_ht};
    return 0;
}
