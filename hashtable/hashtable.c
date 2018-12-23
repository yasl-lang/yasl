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

struct Table *table_new_sized(const int base_size) {
    struct Table *table = malloc(sizeof(struct Table));
    table->base_size = base_size;
    table->size = next_prime(table->base_size);
    table->count = 0;
    table->items = calloc((size_t)table->size, sizeof(Item_t*));
    return table;
}

struct Table *table_new(void) {
    return table_new_sized(HT_BASESIZE);
}
struct Table *table_del(struct Table *table) {
    for (size_t i = 0; i < table->size; i++) {
        Item_t* item = table->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            del_item(item);
        }
    }
    free(table->items);
    free(table);
}

struct RC_Table *rcht_new_sized(const int base_size) {
    struct RC_Table* ht = malloc(sizeof(struct RC_Table));
    ht->table = table_new_sized(base_size);
    ht->rc = rc_new();
    return ht;
}

struct RC_Table* rcht_new() {
    return rcht_new_sized(HT_BASESIZE);
}

void rcht_del(struct RC_Table *hashtable) {
    table_del(hashtable->table);
    rc_del(hashtable->rc);
    free(hashtable);
}

void rcht_del_data(struct RC_Table *hashtable) {
    table_del(hashtable->table);
}

void rcht_del_rc(struct RC_Table *hashtable) {
    rc_del(hashtable->rc);
    free(hashtable);
}

void rcht_del_cstring_cfn(struct RC_Table *hashtable) {
    rcht_del(hashtable);
}

void table_del_string_int(struct Table *table) {
    for (size_t i = 0; i < table->size; i++) {
        Item_t* item = table->items[i];
        if (item != NULL) {
            str_del(item->key->value.sval);
            dec_ref(item->value);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    free(table->items);
    free(table);
}

void rcht_del_string_int(struct RC_Table *hashtable) {
    rc_del(hashtable->rc);
    table_del_string_int(hashtable->table);
    free(hashtable);
}

static void table_resize(struct Table *table, const int base_size) {
    if (base_size < HT_BASESIZE) return;
    struct Table* new_table = table_new_sized(base_size);
    for (size_t i = 0; i < table->size; i++) {
        Item_t* item = table->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            table_insert(new_table, *item->key, *item->value);
        }
    }
    table->base_size = new_table->base_size;
    table->count = new_table->count;

    const size_t tmp_size = table->size;
    table->size = new_table->size;
    new_table->size = tmp_size;

    Item_t** tmp_items = table->items;
    table->items = new_table->items;
    new_table->items = tmp_items;

    table_del(new_table);
}

static void rcht_resize(struct RC_Table *ht, const int base_size) {
    if (base_size < HT_BASESIZE) return;
    struct RC_Table* new_ht = rcht_new_sized(base_size);
    for (size_t i = 0; i < ht->table->size; i++) {
        Item_t* item = ht->table->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            rcht_insert(new_ht, *item->key, *item->value);
        }    
    }
    ht->table->base_size = new_ht->table->base_size;
    ht->table->count = new_ht->table->count;

    const int tmp_size = ht->table->size;
    ht->table->size = new_ht->table->size;
    new_ht->table->size = tmp_size;

    Item_t** tmp_items = ht->table->items;
    ht->table->items = new_ht->table->items;
    new_ht->table->items = tmp_items;

    rcht_del(new_ht);
}

static void table_resize_up(struct Table *table) {
    const size_t new_size = table->base_size * 2;
    table_resize(table, new_size);
}

static void rcht_resize_up(struct RC_Table *ht) {
    const size_t new_size = ht->table->base_size * 2;
    rcht_resize(ht, new_size);
}

static void table_resize_down(struct Table *table) {
    const size_t new_size = table->base_size / 2;
    table_resize(table, new_size);
}

static void rcht_resize_down(struct RC_Table *ht) {
    const size_t new_size = ht->table->base_size / 2;
    rcht_resize(ht, new_size);
}

void table_insert(struct Table *table, const struct YASL_Object key, const struct YASL_Object value) {
    const int load = table->count * 100 / table->size;
    if (load > 70) table_resize_up(table);
    Item_t* item = new_item(key, value);
    int index = get_hash(*item->key, table->size, 0);
    Item_t* curr_item = table->items[index];
    int i = 1;
    while (curr_item != NULL) {
        if (curr_item != &TOMBSTONE) {
            if (!isfalsey(isequal(*curr_item->key, *item->key))) {
                del_item(curr_item);
                table->items[index] = item;
                return;
            }
        }
        index = get_hash(*item->key, table->size, i++);
        curr_item = table->items[index];

    }
    table->items[index] = item;
    table->count++;
}

void rcht_insert(struct RC_Table *hashtable, const struct YASL_Object key, const struct YASL_Object value) {
    table_insert(hashtable->table, key, value);
}

void rcht_insert_literalcstring_cfunction(struct RC_Table *ht, char *key, int (*addr)(struct YASL_State *),
                                          int num_args) {
    String_t *string = str_new_sized(strlen(key), key);
    rcht_insert(ht, YASL_STR(string), YASL_CFN(addr, num_args));
}

void table_insert_string_int(struct Table *table, char *key, int64_t key_len, int64_t val) {
    String_t *string = str_new_sized(key_len, copy_char_buffer(key_len, key));
    table_insert(table,
                (struct YASL_Object) {.type = Y_STR, .value.sval = string},
                (struct YASL_Object) {.type = Y_INT64, .value.ival = val});
}

void rcht_insert_string_int(struct RC_Table *hashtable, char *key, int64_t key_len, int64_t val) {
    String_t *string = str_new_sized(key_len, copy_char_buffer(key_len, key));
    rcht_insert(hashtable,
                (struct YASL_Object) {.type = Y_STR, .value.sval = string},
                (struct YASL_Object) {.type = Y_INT64, .value.ival = val});
}

struct YASL_Object* table_search(const struct Table *const table, const struct YASL_Object key) {
    size_t index = get_hash(key, table->size, 0);
    Item_t* item = table->items[index];
    int i = 1;
    while (item != NULL) {
        if (!isfalsey(isequal(*item->key, key))) {
            return item->value;
        }
        index = get_hash(key, table->size, i++);
        item = table->items[index];
    }
    return NULL;
}

struct YASL_Object *table_search_string_int(const struct Table *const table, char *key, int64_t key_len) {
    String_t *string = str_new_sized(key_len, copy_char_buffer(key_len, key));
    struct YASL_Object object = (struct YASL_Object) { .value.sval = string, .type = Y_STR };

    struct YASL_Object *result = table_search(table, object);
    str_del(string);
    return result;
}

struct YASL_Object* rcht_search(const struct RC_Table *const hashtable, const struct YASL_Object key) {
    return table_search(hashtable->table, key);
}

struct YASL_Object *rcht_search_string_int(const struct RC_Table *const hashtable, char *key, int64_t key_len) {
    String_t *string = str_new_sized(key_len, copy_char_buffer(key_len, key));
    struct YASL_Object object = (struct YASL_Object) { .value.sval = string, .type = Y_STR };

    struct YASL_Object *result = rcht_search(hashtable, object);
    str_del(string);
    return result;

}

void rcht_rm(struct RC_Table *hashtable, struct YASL_Object key) {
    const int load = hashtable->table->count * 100 / hashtable->table->size;
    if (load < 10) rcht_resize_down(hashtable);
    int index = get_hash(key, hashtable->table->size, 0);
    Item_t* item = hashtable->table->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &TOMBSTONE) {
            if (!isfalsey(isequal(*item->key, key))) {
                del_item(item);
                hashtable->table->items[index] = &TOMBSTONE;
            }
        }
        index = get_hash(key, hashtable->table->size, i++);
        item = hashtable->table->items[index];
    }
    hashtable->table->count--;
}

/*
void ht_print(const struct RC_Table *const  ht) {
    ByteBuffer *seen = bb_new(sizeof(int64_t)*2);
    ht_print_h(ht, seen);
}

void ht_print_h(const struct RC_Table *const ht, ByteBuffer* seen) {
    size_t i = 0;
    int64_t *new_seen;
    if (ht->table->count == 0) {
        printf("{}");
        return;
    }
    printf("{");
    Item_t* item = NULL;
    while (i < ht->table->size) {
        item = ht->table->items[i];
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
                bb_intbytes8(seen, ht->table->items[i]->value->value.ival);
                ls_print_h(ht->table->items[i]->value->value.lval, seen);
            }
        } else if (item->value->type == Y_TABLE) {
            if (isvalueinarray(item->value->value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("{...}");
            } else {
                bb_intbytes8(seen, (int64_t)ht);
                bb_intbytes8(seen, ht->table->items[i]->value->value.ival);
                ht_print_h(ht->table->items[i]->value->value.mval, seen);
            }
        } else {
            print(*item->value);
        }
        printf(", ");
        i++;
    }
    printf("\b\b}");
}
*/
