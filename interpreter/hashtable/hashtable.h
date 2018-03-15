#pragma once

#include "../constant/constant.c"
#include <inttypes.h>
#define LEN(v) (*((int64_t*)v.value))

typedef struct {
    Constant* key;
    Constant* value;
} Item_t;

typedef struct {
    int64_t size;
    int64_t base_size;
    int64_t count;
    Item_t** items;
} Hash_t;

void ht_insert(Hash_t* hashtable, Constant key, Constant value);
Constant* ht_search(Hash_t* hashtable, Constant key);
void ht_delete(Hash_t* hashtable, Constant key);
