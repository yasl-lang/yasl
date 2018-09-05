#include <interpreter/YASL_Object/YASL_Object.h>
#include "table_methods.h"
#include "hashtable.h"

int table___get(VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.__get");
    Hash_t* ht = vm_pop(vm).value.mval;
    YASL_Object key = PEEK(vm);
    YASL_Object *result = ht_search(ht, key);
    if (result == NULL) return -1;
    else {
        vm_pop(vm);
        vm_push(vm, *result);
    }
    return 0;
}

int table___set(VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.__set");
    Hash_t* ht = vm_pop(vm).value.mval;
    YASL_Object val = vm_pop(vm);
    YASL_Object key = vm_pop(vm);
    if (yasl_type_equals(key.type, Y_LIST) || yasl_type_equals(key.type, Y_TABLE)) {
        printf("Error: unable to use mutable object of type %x as key.\n", key.type);
        return -1;
    }
    ht_insert(ht, key, val);
    vm_push(vm, val);
    return 0;
}

int table_keys(VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.keys");
    YASL_Object ht = vm_pop(vm);
    List_t* ls = ls_new();
    int64_t i;
    Item_t* item;
    for (i = 0; i < (ht.value.mval)->size; i++) {
        item = (ht.value.mval)->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->key));
        }
    }
    vm_push(vm, YASL_List(ls));
    return 0;
}

int table_values(VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.values");
    YASL_Object ht = vm_pop(vm);
    List_t* ls = ls_new();
    int64_t i;
    Item_t* item;
    for (i = 0; i < (ht.value.mval)->size; i++) {
        item = (ht.value.mval)->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->value));
        }
    }
    vm_push(vm, YASL_List(ls));
    return 0;
}

int table_clone(VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.clone");
    Hash_t* ht = POP(vm).value.mval;
    Hash_t* new_ht = ht_new_sized(ht->base_size);
    int i;
    for (i = 0; i < ht->size; i++) {
        Item_t* item = ht->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ht_insert(new_ht, *item->key, *item->value);
        }
    }
    vm_push(vm, YASL_Table(new_ht));
    return 0;
}
