#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include <interpreter/YASL_string/YASL_string.h>
#include <color.h>
#include <interpreter/refcount/refcount.h>

#define HT_BASESIZE 30

static int hash_function(const struct YASL_Object s, const int a, const int m) {
    long hash = 0;
    if (YASL_ISSTR(s)) {
        const int64_t len_s = yasl_string_len(s.value.sval);
        int i;
        for (i = 0; i < len_s; i++) {
            hash += (long)pow(a, len_s - (i+1)) * ((s.value.sval)->str[i + s.value.sval->start]);
            hash = hash % m;
        }
        return (int)hash;
    } else {  //TODO: make a better hash function for non-strings
        return (int)(((long)pow(a, 8) * s.type) % m);
    }
}

static int get_hash(const struct YASL_Object s, const int num_buckets, const int attempt) {
    const int hash_a = hash_function(s, PRIME_A, num_buckets);
    const int hash_b = hash_function(s, PRIME_B, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

static Item_t* new_item(const struct YASL_Object k, const struct YASL_Object v) {
    Item_t* item = malloc(sizeof(Item_t));
    item->key = malloc(sizeof(struct YASL_Object));
    item->value = malloc(sizeof(struct YASL_Object));
    item->key->type    = k.type;
    item->key->value   = k.value;
    item->value->type  = v.type;
    item->value->value = v.value;
    inc_ref(item->value);
    inc_ref(item->key);
    /*if (yasl_type_equals(v.type, Y_STR)) {
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
    dec_ref(item->key);
    dec_ref(item->value);
    free(item->key);
    free(item->value);
    free(item);
}

struct RC_Table* ht_new_sized(const int base_size) {
    struct RC_Table* ht = malloc(sizeof(struct RC_Table));
    ht->base_size = base_size;
    ht->size = next_prime(ht->base_size);
    ht->count = 0;
    ht->items = calloc((size_t)ht->size, sizeof(Item_t*));
    ht->rc = rc_new();
    return ht;
}

struct RC_Table* ht_new() {
    return ht_new_sized(HT_BASESIZE);
}

void del_hash(struct RC_Table* hashtable) {
    for (size_t i = 0; i < hashtable->size; i++) {
        Item_t* item = hashtable->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            dec_ref(item->key);
            dec_ref(item->value);
            del_item(item);
        }
    }
    free(hashtable->items);
    rc_del(hashtable->rc);
    free(hashtable);
}

void ht_del_data(struct RC_Table* hashtable) {
    for (size_t i = 0; i < hashtable->size; i++) {
        Item_t* item = hashtable->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            // dec_ref(item->key);
            // dec_ref(item->value);
            del_item(item);
        }
    }
    free(hashtable->items);
}

void ht_del_rc(struct RC_Table* hashtable) {
    rc_del(hashtable->rc);
    free(hashtable);
}

void ht_del_cstring_cfn(struct RC_Table *hashtable) {
    for (size_t i = 0; i < hashtable->size; i++) {
        Item_t* item = hashtable->items[i];
        if (item != NULL) {
            // printf("%d\n", item->value->value.cval->rc->refs);
            // printf("%d\n", item->key->value.sval->rc->refs);
            dec_ref(item->key);
            dec_ref(item->value);
            // str_del(item->key->value.sval);
            //dec_ref(item->value);
            //free(item->value->value.cval);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    rc_del(hashtable->rc);
    free(hashtable->items);
    free(hashtable);
}

void ht_del_string_int(struct RC_Table *hashtable) {
    for (size_t i = 0; i < hashtable->size; i++) {
        Item_t* item = hashtable->items[i];
        if (item != NULL) {
            str_del(item->key->value.sval);
            dec_ref(item->value);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    rc_del(hashtable->rc);
    free(hashtable->items);
    free(hashtable);
}

static void ht_resize(struct RC_Table* ht, const int base_size) {
    if (base_size < HT_BASESIZE) return;
    struct RC_Table* new_ht = ht_new_sized(base_size);
    for (size_t i = 0; i < ht->size; i++) {
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

static void ht_resize_up(struct RC_Table* ht) {
    puts("resize up");
    const int new_size = ht->base_size * 2;
    ht_resize(ht, new_size);
}

static void ht_resize_down(struct RC_Table* ht) {
    puts("resize down");
    const int new_size = ht->base_size / 2;
    ht_resize(ht, new_size);
}

void ht_insert(struct RC_Table* hashtable, const struct YASL_Object key, const struct YASL_Object value) {
    const int load = hashtable->count * 100 / hashtable->size;
    if (load > 70) ht_resize_up(hashtable);
    Item_t* item = new_item(key, value);
    int index = get_hash(*item->key, hashtable->size, 0);
    Item_t* curr_item = hashtable->items[index];
    int i = 1;
    while (curr_item != NULL) {
        if (curr_item != &TOMBSTONE) {
            if (!isfalsey(isequal(*curr_item->key, *item->key))) {
                // dec_ref(item->key);
                // dec_ref(item->value);
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

void ht_insert_literalcstring_cfunction(struct RC_Table *ht, char *key, int (*addr)(struct YASL_State *), int num_args) {
    String_t *string = str_new_sized(strlen(key), key);
    ht_insert(ht, YASL_STR(string), YASL_CFN(addr, num_args));
}

void ht_insert_string_int(struct RC_Table *hashtable, char *key, int64_t key_len, int64_t val) {
    String_t *string = str_new_sized(key_len, copy_char_buffer(key_len, key));
    ht_insert(hashtable,
              (struct YASL_Object) { .type = Y_STR, .value.sval = string},
              (struct YASL_Object) { .type = Y_INT64, .value.ival = val});
}

struct YASL_Object* ht_search(const struct RC_Table *const hashtable, const struct YASL_Object key) {
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

struct YASL_Object *ht_search_string_int(const struct RC_Table *const hashtable, char *key, int64_t key_len) {
    String_t *string = str_new_sized(key_len, copy_char_buffer(key_len, key));
    struct YASL_Object object = (struct YASL_Object) { .value.sval = string, .type = Y_STR };

    struct YASL_Object *result = ht_search(hashtable, object);
    str_del(string);
    return result;

}

void ht_rm(struct RC_Table *hashtable, struct YASL_Object key) {
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

void ht_print(const struct RC_Table *const  ht) {
    ByteBuffer *seen = bb_new(sizeof(int64_t)*2);
    ht_print_h(ht, seen);
}

void ht_print_h(const struct RC_Table *const ht, ByteBuffer* seen) {
    size_t i = 0;
    int64_t *new_seen;
    if (ht->count == 0) {
        printf("{}");
        return;
    }
    printf("{");
    Item_t* item = NULL;
    while (i < ht->size) {
        item = ht->items[i];
        if (item == &TOMBSTONE || item == NULL) {
            i++;
            continue;
        }
        print(*item->key);
        printf(":");
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
                printf("{...}");
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
    printf("\b\b}");
}
