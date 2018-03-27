#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vtable.h"
#include "../prime/prime.h"
#define VT_BASESIZE 60

static VItem_t VTOMBSTONE = {-1, 0};

static int vhash_function(const int64_t s, const int64_t a, const int64_t m) {
    return (s * a ) % m;
}

static int get_vhash(const int64_t s, const int64_t num_buckets, const int64_t attempt) {
    const int hash_a = vhash_function(s, PRIME_A, num_buckets);
    const int hash_b = vhash_function(s, PRIME_B, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

static VItem_t* new_vitem(const int64_t k, const int64_t v) {
    VItem_t* item = malloc(sizeof(VItem_t));
    item->key = k;
    item->value = v;
    return item;
}

static void del_vitem(VItem_t* item) {
    free(item);
}

static VTable_t* new_sized_vtable(const int64_t base_size) {
    VTable_t* vt = malloc(sizeof(VTable_t));
    vt->parent = NULL;
    vt->base_size = base_size;
    vt->size = next_prime(vt->base_size);
    vt->count = 0;
    vt->items = calloc((size_t)vt->size, sizeof(VTable_t*));
    return vt;
}

VTable_t* new_vtable() {
    return new_sized_vtable(VT_BASESIZE);
}

void del_vtable(VTable_t* vtable) {
    int i;
    for (i = 0; i < vtable->size; i++) {
        VItem_t* item = vtable->items[i];
        if (item != NULL) {
            del_vitem(item);
        }
    }
    free(vtable->items);
    free(vtable);
}

static void vt_resize(VTable_t* vt, const int64_t base_size) {
    if (base_size < VT_BASESIZE) return;
    VTable_t* new_vt = new_sized_vtable(base_size);
    int64_t i;
    for (i = 0; i < vt->size; i++) {
        VItem_t* item = vt->items[i];
        if (item != NULL && item != &VTOMBSTONE) {
            vt_insert(new_vt, item->key, item->value);
        }
    }
    vt->base_size = new_vt->base_size;
    vt->count = new_vt->count;

    const int64_t tmp_size = vt->size;
    vt->size = new_vt->size;
    new_vt->size = tmp_size;

    VItem_t** tmp_items = vt->items;
    vt->items = new_vt->items;
    new_vt->items = tmp_items;

    del_vtable(new_vt);
}

static void vt_resize_up(VTable_t* vt) {
    puts("resize up");
    const int64_t new_size = vt->base_size * 2;
    vt_resize(vt, new_size);
}

static void vt_resize_down(VTable_t* vt) {
    puts("resize down");
    const int64_t new_size = vt->base_size / 2;
    vt_resize(vt, new_size);
}

void vt_insert(VTable_t* vtable, const int64_t key, const int64_t value) {
    const int load = vtable->count * 100 / vtable->size;
    if (load > 70) vt_resize_up(vtable);
    VItem_t* item = new_vitem(key, value);
    int64_t index = get_vhash(item->key, vtable->size, 0);
    VItem_t* curr_item = vtable->items[index];
    int64_t i = 1;
    while (curr_item != NULL) {
        if (curr_item != &VTOMBSTONE) {
            if (curr_item->key == item->key) {
                del_vitem(curr_item);
                vtable->items[index] = item;
                return;
            }
        }
        index = get_vhash(item->key, vtable->size, i++);
        curr_item = vtable->items[index];

    }
    vtable->items[index] = item;
    vtable->count++;
}

int64_t vt_search(VTable_t* vtable, const int64_t key) { //TODO: make so that search continues in parent.
    int64_t index = get_vhash(key, vtable->size, 0);
    VItem_t* item = vtable->items[index];
    int64_t i = 1;
    while (item != NULL) {
        if (item->key == key) {
            return item->value;
        }
        index = get_vhash(key, vtable->size, i++);
        item = vtable->items[index];
    }
    return -1;
}

void vt_delete(VTable_t* vtable, const int64_t key) {
    const int load = vtable->count * 100 / vtable->size;
    if (load < 10) vt_resize_down(vtable);
    int index = get_vhash(key, vtable->size, 0);
    VItem_t* item = vtable->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &VTOMBSTONE) {
            if (item->key == key) {
                del_vitem(item);
                vtable->items[index] = &VTOMBSTONE;
            }
        }
        index = get_vhash(key, vtable->size, i++);
        item = vtable->items[index];
    }
    vtable->count--;
}

