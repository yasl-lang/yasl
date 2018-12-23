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
    struct Table *table;
};

static Item_t TOMBSTONE = {0, 0};

struct Table *table_new(void);
void table_insert(struct Table *table, const struct YASL_Object key, const struct YASL_Object value);
void table_insert_string_int(struct Table *table, char *key, int64_t key_len, int64_t val);
struct YASL_Object* table_search(const struct Table *const table, const struct YASL_Object key);
struct YASL_Object *table_search_string_int(const struct Table *const table, char *key, int64_t key_len);

struct RC_Table* rcht_new(void);
struct RC_Table* rcht_new_sized(const int base_size);
void rcht_del_data(struct RC_Table *hashtable);
void rcht_del_rc(struct RC_Table *hashtable);
void rcht_del_cstring_cfn(struct RC_Table *hashtable);
void rcht_del_string_int(struct RC_Table *hashtable);
void rcht_insert(struct RC_Table *hashtable, const struct YASL_Object key, const struct YASL_Object value);
void rcht_insert_literalcstring_cfunction(struct RC_Table *ht, char *key, int (*addr)(struct YASL_State *),
                                          int num_args);
void rcht_insert_string_int(struct RC_Table *hashtable, char *key, int64_t key_len, int64_t val);
struct YASL_Object* rcht_search(const struct RC_Table *const hashtable, const struct YASL_Object key);
struct YASL_Object *rcht_search_string_int(const struct RC_Table *const hashtable, char *key, int64_t key_len);
void rcht_rm(struct RC_Table *hashtable, struct YASL_Object key);
void ht_print(const struct RC_Table *const ht);
void ht_print_h(const struct RC_Table *const ht, ByteBuffer* seen);
