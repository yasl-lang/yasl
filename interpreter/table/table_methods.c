#include <interpreter/YASL_Object/YASL_Object.h>
#include "table_methods.h"
#include "hashtable.h"

int table___get(struct VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.__get");
    Hash_t* ht = vm_pop(vm).value.mval;
    struct YASL_Object key = PEEK(vm);
    struct YASL_Object *result = ht_search(ht, key);
    if (result == NULL) return -1;
    else {
        vm_pop(vm);
        vm_push(vm, result);
    }
    return 0;
}

int table___set(struct VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.__set");
    Hash_t* ht = vm_pop(vm).value.mval;
    struct YASL_Object val = vm_pop(vm);
    struct YASL_Object key = vm_pop(vm);
    if (yasl_type_equals(key.type, Y_LIST) || yasl_type_equals(key.type, Y_TABLE)) {
        printf("Error: unable to use mutable object of type %x as key.\n", key.type);
        return -1;
    }
    ht_insert(ht, key, val);
    vm_push(vm, &val);
    return 0;
}

int table_keys(struct VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.keys");
    struct YASL_Object ht = vm_pop(vm);
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

int table_values(struct VM *vm) {
    ASSERT_TYPE(vm, Y_TABLE, "table.values");
    struct YASL_Object ht = vm_pop(vm);
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

int table_clone(struct VM *vm) {
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
    struct YASL_Object *hashtable = malloc(sizeof(struct YASL_Object));
    hashtable->type = Y_TABLE;
    hashtable->value.mval = new_ht;
    vm_push(vm, hashtable);
    return 0;
}
