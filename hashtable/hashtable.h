#pragma once

#include <inttypes.h>

#include "prime.h"
#include "YASL_Object.h"
#include "list.h"

#define LEN(v) (*((int64_t*)(v).value))

typedef struct {
    struct YASL_Object* key;
    struct YASL_Object* value;
} Item_t;

struct Table {
    size_t size;
    size_t base_size;
    size_t count;
    Item_t **items;
};

struct RC_Table {
    struct RC *rc;
    struct Table table;
};

static Item_t TOMBSTONE = {0, 0};

struct RC_Table* ht_new(void);
struct RC_Table* ht_new_sized(const int base_size);
void ht_del_data(struct RC_Table *hashtable);
void ht_del_rc(struct RC_Table *hashtable);
void ht_del_cstring_cfn(struct RC_Table *hashtable);
void ht_del_string_int(struct RC_Table *hashtable);
void ht_insert(struct RC_Table *const hashtable, struct YASL_Object key, struct YASL_Object value);
void ht_insert_literalcstring_cfunction(struct RC_Table *ht, char *key, int (*addr)(struct YASL_State *), int num_args);
void ht_insert_string_int(struct RC_Table *const hashtable, char *key, int64_t key_len, int64_t val);
struct YASL_Object* ht_search(const struct RC_Table *const hashtable, struct YASL_Object key);
struct YASL_Object *ht_search_string_int(const struct RC_Table *const hashtable, char *key, int64_t key_len);
void ht_rm(struct RC_Table *hashtable, struct YASL_Object key);
void ht_print(const struct RC_Table *const ht);
void ht_print_h(const struct RC_Table *const ht, ByteBuffer* seen);
