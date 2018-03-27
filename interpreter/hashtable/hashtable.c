#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hashtable.h"
#include "../constant/constant.h"
#include "../prime/prime.h"
#define HT_BASESIZE 60

static int hash_function(const Constant s, const int a, const int m) {
    long hash = 0;
    if (s.type == STR8) {
        const int len_s = ((String_t*)s.value)->length;
        int i;
        for (i = 0; i < len_s; i++) {
            hash += (long)pow(a, len_s - (i+1)) * (((String_t*)s.value)->str[i]);
            hash = hash % m;
        }
        return (int)hash;
    } else {  //TODO: make a better hash function for non-strings
        return (int)(((long)pow(a, 8) * s.type) % m);
    }
}

static int get_hash(const Constant s, const int num_buckets, const int attempt) {
    const int hash_a = hash_function(s, PRIME_A, num_buckets);
    const int hash_b = hash_function(s, PRIME_B, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

static Item_t* new_item(const Constant k, const Constant v) {
    Item_t* item = malloc(sizeof(Item_t));
    item->key = malloc(sizeof(Constant));
    item->value = malloc(sizeof(Constant));
    item->key->type    = k.type;
    item->key->value   = k.value;
    item->value->type  = v.type;
    item->value->value = v.value;
    /*if (v.type == STR8) {
        item->key.type = v.type;
        int64_t len_v = *((int64_t*)v->value);
        //printf("%" PRId64 "\n", len_v);
    }
    item->key->type = k->type;     // TODO: make so that this does deep copy
    item->key->value = k->value;
    item->value->type = v->type;
    item->value->value = v->value;   // TODO: make so that this does deep copy */
    return item;
}

static void del_item(Item_t* item) {
    free(item->key);
    free(item->value);
    free(item);
}

static Hash_t* new_sized_hash(const int base_size) {
    Hash_t* ht = malloc(sizeof(Hash_t));
    ht->base_size = base_size;
    ht->size = next_prime(ht->base_size);
    ht->count = 0;
    ht->items = calloc((size_t)ht->size, sizeof(Hash_t*));
    return ht;
}

Hash_t* new_hash() {
    return new_sized_hash(HT_BASESIZE);
}

void del_hash(Hash_t* hashtable) {
    int i;
    for (i = 0; i < hashtable->size; i++) {
        Item_t* item = hashtable->items[i];
        if (item != NULL) {
            del_item(item);
        }
    }
    free(hashtable->items);
    free(hashtable);
}

static void ht_resize(Hash_t* ht, const int base_size) {
    if (base_size < HT_BASESIZE) return;
    Hash_t* new_ht = new_sized_hash(base_size);
    int i;
    for (i = 0; i < ht->size; i++) {
        Item_t* item = ht->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ht_insert(new_ht, *item->key, *item->value);
        }    
    }
    ht->base_size = new_ht->base_size;
    ht->count = new_ht->count;

    const int tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;

    Item_t** tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;

    del_hash(new_ht);
}

static void ht_resize_up(Hash_t* ht) {
    puts("resize up");
    const int new_size = ht->base_size * 2;
    ht_resize(ht, new_size);
}

static void ht_resize_down(Hash_t* ht) {
    puts("resize down");
    const int new_size = ht->base_size / 2;
    ht_resize(ht, new_size);
}

void ht_insert(Hash_t* hashtable, const Constant key, const Constant value) {
    const int load = hashtable->count * 100 / hashtable->size;
    if (load > 70) ht_resize_up(hashtable);
    Item_t* item = new_item(key, value);
    int index = get_hash(*item->key, hashtable->size, 0);
    Item_t* curr_item = hashtable->items[index];
    int i = 1;
    while (curr_item != NULL) {
        if (curr_item != &TOMBSTONE) {
            //puts("checking equality");
            if (!FALSEY(isequal(*curr_item->key, *item->key))) {
                del_item(curr_item);
                hashtable->items[index] = item;
                return;
            }
        }
        index = get_hash(*item->key, hashtable->size, i++);
        curr_item = hashtable->items[index];

    }
    hashtable->items[index] = item;
    hashtable->count++;
}

Constant* ht_search(Hash_t* hashtable, const Constant key) {
    //puts("searching");
    int index = get_hash(key, hashtable->size, 0);
    Item_t* item = hashtable->items[index];
    int i = 1;
    while (item != NULL) {
        //puts("goooo");
        //print(*item->key);
        //print(key);
        if (!FALSEY(isequal(*item->key, key))) {
            return item->value;
        }
        index = get_hash(key, hashtable->size, i++);
        item = hashtable->items[index];
    }
    //puts("return null");
    return NULL;
}

void ht_delete(Hash_t* hashtable, const Constant key) {
    const int load = hashtable->count * 100 / hashtable->size;
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
}
