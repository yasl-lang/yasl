#include "hashtable.h"
#include "refcountptr.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include <interpreter/YASL_string/YASL_string.h>

#define HT_BASESIZE 30

static int hash_function(const YASL_Object s, const int a, const int m) {
    long hash = 0;
    if (s.type == Y_STR) {
        const int64_t len_s = yasl_string_len(s.value.sval);
        int i;
        for (i = 0; i < len_s; i++) {
            hash += (long)pow(a, len_s - (i+1)) * ((s.value.sval)->str.ptr[i + s.value.sval->start]);
            hash = hash % m;
        }
        return (int)hash;
    } else {  //TODO: make a better hash function for non-strings
        return (int)(((long)pow(a, 8) * s.type) % m);
    }
}

static int get_hash(const YASL_Object s, const int num_buckets, const int attempt) {
    const int hash_a = hash_function(s, PRIME_A, num_buckets);
    const int hash_b = hash_function(s, PRIME_B, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

static Item_t* new_item(const YASL_Object k, const YASL_Object v) {
    Item_t* item = malloc(sizeof(Item_t));
    item->key = malloc(sizeof(YASL_Object));
    item->value = malloc(sizeof(YASL_Object));
    item->key->type    = k.type;
    item->key->value   = k.value;
    item->value->type  = v.type;
    item->value->value = v.value;
    /*if (v.type == Y_STR) {
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

Hash_t* ht_new_sized(const int base_size) {
    Hash_t* ht = malloc(sizeof(Hash_t));
    ht->base_size = base_size;
    ht->size = next_prime(ht->base_size);
    ht->count = 0;
    ht->items = calloc((size_t)ht->size, sizeof(Hash_t*));
    return ht;
}

Hash_t* ht_new() {
    return ht_new_sized(HT_BASESIZE);
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

void ht_del_string_int(Hash_t *hashtable) {
    int i;
    for (i = 0; i < hashtable->size; i++) {
        Item_t* item = hashtable->items[i];
        if (item != NULL) {
            str_del(item->key->value.sval);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    free(hashtable->items);
    free(hashtable);
}

static void ht_resize(Hash_t* ht, const int base_size) {
    if (base_size < HT_BASESIZE) return;
    Hash_t* new_ht = ht_new_sized(base_size);
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

void ht_insert(Hash_t* hashtable, const YASL_Object key, const YASL_Object value) {
    const int load = hashtable->count * 100 / hashtable->size;
    if (load > 70) ht_resize_up(hashtable);
    Item_t* item = new_item(key, value);
    int index = get_hash(*item->key, hashtable->size, 0);
    Item_t* curr_item = hashtable->items[index];
    int i = 1;
    while (curr_item != NULL) {
        if (curr_item != &TOMBSTONE) {
            //puts("checking equality");
            if (!isfalsey(isequal(*curr_item->key, *item->key))) {
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

void ht_insert_string_int(Hash_t *hashtable, char *key, int64_t key_len, int64_t val) {
    String_t *string = malloc(sizeof(String_t));
    char *tmp = malloc(key_len);
    memcpy(tmp, key, key_len);
    string->str = rcptr_new(tmp);
    string->start = 0;
    string->end = key_len;
    ht_insert(hashtable,
              (YASL_Object) { .type = Y_STR, .value.sval = string},
              (YASL_Object) { .type = Y_BFN, .value.ival = val});
}

YASL_Object* ht_search(const Hash_t *const hashtable, const YASL_Object key) {
    int index = get_hash(key, hashtable->size, 0);
    Item_t* item = hashtable->items[index];
    int i = 1;
    while (item != NULL) {
        if (!isfalsey(isequal(*item->key, key))) {
            return item->value;
        }
        index = get_hash(key, hashtable->size, i++);
        item = hashtable->items[index];
    }
    return NULL;
}

YASL_Object *ht_search_string_int(const Hash_t *const hashtable, char *key, int64_t key_len) {
    String_t *string = malloc(sizeof(String_t));
    char *tmp = malloc(key_len);
    memcpy(tmp, key, key_len);
    string->str = rcptr_new(tmp);
    string->start = 0;
    string->end = key_len;
    YASL_Object object = (YASL_Object) { .value.sval = string, .type = Y_STR };

    YASL_Object *result = ht_search(hashtable, object);
    str_del(string);
    return result;

}

void ht_rm(Hash_t *hashtable, YASL_Object key) {
    const int load = hashtable->count * 100 / hashtable->size;
    if (load < 10) ht_resize_down(hashtable);
    int index = get_hash(key, hashtable->size, 0);
    Item_t* item = hashtable->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &TOMBSTONE) {
            if (!isfalsey(isequal(*item->key, key))) {
                del_item(item);
                hashtable->items[index] = &TOMBSTONE;
            }
        }
        index = get_hash(key, hashtable->size, i++);
        item = hashtable->items[index];
    }
    hashtable->count--;
}

void ht_print(const Hash_t *const  ht) {
    ByteBuffer *seen = bb_new(sizeof(int64_t)*2);
    ht_print_h(ht, seen);
}

void ht_print_h(const Hash_t *const ht, ByteBuffer* seen) {
    int i = 0;
    int64_t *new_seen;
    if (ht->count == 0) {
        printf("[->]");
        return;
    }
    printf("[");
    Item_t* item = NULL;
    while (i < ht->size) {
        item = ht->items[i];
        if (item == &TOMBSTONE || item == NULL) {
            i++;
            continue;
        }
        print(*item->key);
        printf("->");
        if (item->value->type == Y_LIST) {
            if (isvalueinarray(item->value->value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("[...]");
            } else {
                bb_intbytes8(seen, (int64_t)ht);
                bb_intbytes8(seen, ht->items[i]->value->value.ival);
                ls_print_h(ht->items[i]->value->value.lval, seen);
            }
        } else if (item->value->type == Y_TABLE) {
            if (isvalueinarray(item->value->value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("[...->...]");
            } else {
                bb_intbytes8(seen, (int64_t)ht);
                bb_intbytes8(seen, ht->items[i]->value->value.ival);
                ht_print_h(ht->items[i]->value->value.mval, seen);
            }
        } else {
            print(*item->value);
        }
        printf(", ");
        i++;
    }
    printf("\b\b]");
}