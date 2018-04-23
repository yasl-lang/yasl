#include "hash_methods.h"

int map___get(VM *vm) {
    Hash_t* ht = (Hash_t*)POP(vm).value;
    Constant key = POP(vm);
    PUSH(vm, *ht_search(ht, key));
    return 0;
}

int map___set(VM *vm) {
    Hash_t* ht = (Hash_t*)POP(vm).value;
    Constant val = POP(vm);
    Constant key = POP(vm);
    if (key.type == LIST || key.type == MAP) {
        printf("Error: unable to use mutable object of type %x as key.\n", key.type);
        return -1;
    }
    ht_insert(ht, key, val);
    PUSH(vm, val);
    return 0;
}

int map_keys(VM* vm) {
    Constant ht = POP(vm);
    List_t* ls = new_list();
    int64_t i;
    Item_t* item;
    for (i = 0; i < ((Hash_t*)ht.value)->size; i++) {
        item = ((Hash_t*)ht.value)->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->key));
        }
    }
    vm->stack[++vm->sp] = (Constant) {LIST, (int64_t)ls};
    return 0;
}

int map_values(VM* vm) {
    Constant ht = POP(vm);
    List_t* ls = new_list();
    int64_t i;
    Item_t* item;
    for (i = 0; i < ((Hash_t*)ht.value)->size; i++) {
        item = ((Hash_t*)ht.value)->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls, *(item->value));
        }
    }
    vm->stack[++vm->sp] = (Constant) {LIST, (int64_t)ls};
    return 0;
}
