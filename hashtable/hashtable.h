#pragma once

#include "../prime/prime.h"
#include "../interpreter/YASL_Object/YASL_Object.h"
#include "../interpreter/list/list.h"
#include <inttypes.h>

#define LEN(v) (*((int64_t*)v.value))

typedef struct {
    struct YASL_Object* key;
    struct YASL_Object* value;
} Item_t;

typedef struct Hash_s {
    RefCount *rc;
    size_t size;
    size_t base_size;
    size_t count;
    Item_t** items;
} Hash_t;

static Item_t TOMBSTONE = {0, 0};

Hash_t* ht_new(void);
Hash_t* ht_new_sized(const int base_size);
void ht_del_data(Hash_t *hashtable);
void ht_del_rc(Hash_t *hashtable);
void ht_del_cstring_cfn(Hash_t *hashtable);
void ht_del_string_int(Hash_t *hashtable);
void ht_insert(Hash_t *const hashtable, struct YASL_Object key, struct YASL_Object value);
void ht_insert_literalcstring_cfunction(Hash_t *ht, char *key, int (*addr)(struct YASL_State *), int num_args);
void ht_insert_string_int(Hash_t *const hashtable, char *key, int64_t key_len, int64_t val);
struct YASL_Object* ht_search(const Hash_t *const hashtable, struct YASL_Object key);
struct YASL_Object *ht_search_string_int(const Hash_t *const hashtable, char *key, int64_t key_len);
void ht_rm(Hash_t *hashtable, struct YASL_Object key);
void ht_print(const Hash_t *const ht);
void ht_print_h(const Hash_t *const ht, ByteBuffer* seen);
