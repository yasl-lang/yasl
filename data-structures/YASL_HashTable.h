#pragma once

#include "interpreter/YASL_Object.h"

#define HT_BASESIZE 30

#define FOR_TABLE(i, item, table) struct YASL_HashTable_Item *item; for (size_t i = 0; i < (table)->size; i++) \
                                                  if (item = &table->items[i], item->key.type != Y_END && !YASL_ISUNDEF(item->key))

struct YASL_HashTable_Item {
	struct YASL_Object key;
	struct YASL_Object value;
};

struct YASL_HashTable {
	size_t size;
	size_t base_size;
	size_t count;
	struct YASL_HashTable_Item *items;
};

extern struct YASL_HashTable_Item TOMBSTONE;

void del_item(struct YASL_HashTable_Item *const item);

struct YASL_HashTable *table_new(void);
void table_insert(struct YASL_HashTable *const table, const struct YASL_Object key, const struct YASL_Object value);
void table_insert_string_int(struct YASL_HashTable *const table, const char *const key, const size_t key_len, const int64_t val);
void table_insert_literalcstring_cfunction(struct YASL_HashTable *const ht, const char *key, int (*addr)(struct YASL_State *), const int num_args);
struct YASL_Object table_search(const struct YASL_HashTable *const table, const struct YASL_Object key);
struct YASL_Object table_search_string_int(const struct YASL_HashTable *const table, const char *const key, const size_t key_len);
void table_rm(struct YASL_HashTable *const table, struct YASL_Object key);
void table_del(struct YASL_HashTable *const table);
void table_del_string_int(struct YASL_HashTable *const table);

struct RC_UserData* rcht_new(void);
struct RC_UserData* rcht_new_sized(const size_t base_size);
void rcht_del(struct RC_UserData *const hashtable);
void rcht_del_data(void *const hashtable);
void rcht_del_cstring_cfn(struct RC_UserData *const hashtable);
