#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "list.h"
#include "../constant/constant.h"
#define LS_BASESIZE 4

Constant TOMBSTONE = (Constant) {0xF0, 0};

static List_t* new_sized_list(const int base_size) {
    List_t* ls = malloc(sizeof(List_t));
    ls->size = base_size;
    ls->count = 0;
    ls->items = malloc(sizeof(Constant)*ls->size);
    return ls;
}

List_t* new_list() {
    return new_sized_list(LS_BASESIZE);
}

void del_list(List_t* ls) {
    free(ls->items);
    free(ls);
}

static void ls_resize(List_t* ls, const int base_size) {
    if (base_size < LS_BASESIZE) return;
    List_t* new_ls = new_sized_list(base_size);
    int i;
    for (i = 0; i < ls->size; i++) {
        //printf("ls->items[i].value: %d\n", (int)ls->items[i].value);
        new_ls->items[i] = ls->items[i];
    }
    ls->size = new_ls->size;
    //ls->count = new_ls->count;

    Constant* tmp_items = ls->items;
    ls->items = new_ls->items;
    new_ls->items = tmp_items;

    del_list(new_ls);
}

static void ls_resize_up(List_t* ls) {
    //puts("resize up");
    const int new_size = ls->size * 2;
    ls_resize(ls, new_size);
    //printf("new size is: %d\n", ls->size);
}

static void ls_resize_down(List_t* ls) {
    //puts("resize down");
    const int new_size = ls->size / 2;
    ls_resize(ls, new_size);
    //printf("new size is: %d\n", ls->size);
}

void ls_insert(List_t* ls, const int64_t index, const Constant value) {
    if (ls->count >= ls->size) ls_resize_up(ls);
    ls->items[index] = value;
    ls->count++;
}

void ls_append(List_t* ls, const Constant value) {
    if (ls->count >= ls->size) ls_resize_up(ls);
    ls->items[ls->count++] = value;
    //ls->count++;
}

Constant ls_search(List_t* ls, int64_t index) {
    if (index < 0 || index >= ls->size) return (Constant) {0, 0}; // TODO: proper error value
    return ls->items[index];
}

void ls_delete(List_t* hashtable, int64_t index) {
    /*const int load = hashtable->count * 100 / hashtable->size;
    if (load < 10) ht_resize_down(hashtable);
    int index = get_hash(key, hashtable->size, 0);
    Item_t* item = hashtable->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &TOMBSTONE) {
            if (!FALSEY(isequal(*item->key, key))) {
                del_item(item);
                hashtable->items[index] = &TOMBSTONE;
            }
        }
        index = get_hash(key, hashtable->size, i++);
        item = hashtable->items[index];
    }
    hashtable->count--;
    */
}
