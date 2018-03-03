#pragma once

#include "../constant/constant.c"
#define LEN(v) (*((int64_t*)v.value))

typedef struct {
    Constant* key;
    Constant* value;
} Item_t;

typedef struct {
    int size;
    int base_size;
    int count;
    Item_t** items;
} Hash_t;

void ht_insert(Hash_t* hashtable, Constant key, Constant value);
Constant* ht_search(Hash_t* hashtable, Constant key);
void ht_delete(Hash_t* hashtable, Constant key);
