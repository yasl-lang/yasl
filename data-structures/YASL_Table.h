#ifndef YASL_YASL_TABLE_H_
#define YASL_YASL_TABLE_H_

#include "interpreter/YASL_Object.h"
#include "util/prime.h"
#include "yasl_include.h"

#define TABLE_BASESIZE 30

#define FOR_TABLE(i, item, table) struct YASL_Table_Item *item; for (size_t i = 0; i < (table)->size; i++) \
                                                  if (item = &(table)->items[i], item->key.type != Y_END && !obj_isundef(&item->value))


#define NEW_TABLE() ((struct YASL_Table){\
	.size = next_prime(TABLE_BASESIZE),\
	.base_size = TABLE_BASESIZE,\
	.count = 0,\
	.items = (struct YASL_Table_Item *)calloc((size_t) next_prime(TABLE_BASESIZE), sizeof(struct YASL_Table_Item))\
})

#define DEL_TABLE(table) do {\
        FOR_TABLE(i, item, table) {\
                del_item(item);\
        }\
        free((table)->items);\
} while (0)


struct YASL_Table_Item {
	struct YASL_Object key;
	struct YASL_Object value;
};

struct YASL_Table {
	size_t size;
	size_t base_size;
	size_t count;
	struct YASL_Table_Item *items;
};

extern struct YASL_Table_Item TOMBSTONE;

void del_item(struct YASL_Table_Item *const item);

struct YASL_Table *YASL_Table_new(void);
void YASL_Table_del(struct YASL_Table *const table);
bool YASL_Table_insert(struct YASL_Table *const table, const struct YASL_Object key, const struct YASL_Object value) /* YASL_WARN_UNUSED */;
void YASL_Table_insert_fast(struct YASL_Table *const table, const struct YASL_Object key, const struct YASL_Object value);
void YASL_Table_insert_string_int(struct YASL_Table *const table, const char *const key, const size_t key_len,
				  const int64_t val);
void YASL_Table_insert_literalcstring_cfunction(struct YASL_Table *const ht, const char *key,
						YASL_cfn addr, const int num_args);
yasl_int YASL_Table_length(const struct YASL_Table *const ht);
struct YASL_Object YASL_Table_search(const struct YASL_Table *const table, const struct YASL_Object key);
struct YASL_Object YASL_Table_search_string_int(const struct YASL_Table *const table, const char *const key,
						const size_t key_len);
void YASL_Table_rm(struct YASL_Table *const table, const struct YASL_Object key);

struct RC_UserData* rcht_new(void);
struct RC_UserData* rcht_new_sized(const size_t base_size);
void rcht_del(struct RC_UserData *const hashtable);
void rcht_del_data(void *const hashtable);
void rcht_del_cstring_cfn(struct RC_UserData *const hashtable);

#endif
