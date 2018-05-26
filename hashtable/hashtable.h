#pragma once

#include "../prime/prime.h"
#include "../interpreter/YASL_Object/YASL_Object.h"
#include "../interpreter/list/list.h"
#include <inttypes.h>

#define LEN(v) (*((int64_t*)v.value))

typedef struct {
    YASL_Object* key;
    YASL_Object* value;
} Item_t;

static Item_t TOMBSTONE = {0, 0};

typedef struct {
    int64_t size;
    int64_t base_size;
    int64_t count;
    Item_t** items;
} Hash_t;

Hash_t* new_hash(void);
void ht_insert(Hash_t* hashtable, YASL_Object key, YASL_Object value);
YASL_Object* ht_search(Hash_t* hashtable, YASL_Object key);
void ht_delete(Hash_t* hashtable, YASL_Object key);
void ht_print(Hash_t* ht);
void ht_print_h(Hash_t* ht, int64_t* seen, int seen_size);
