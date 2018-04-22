#include "hash_methods.h"

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
