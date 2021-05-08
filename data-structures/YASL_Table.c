#include "YASL_Table.h"

#include <string.h>

#include "data-structures/YASL_String.h"
#include "debug.h"
#include "hash_function.h"
#include "interpreter/refcount.h"
#include "interpreter/YASL_Object.h"
#include "interpreter/userdata.h"
#include "yasl_error.h"

const char *const TABLE_NAME = "table";

struct YASL_Table_Item TOMBSTONE = { { Y_END, { Y_END } }, { Y_END, { Y_END } } };

static struct YASL_Table_Item new_item(const struct YASL_Object k, const struct YASL_Object v) {
	struct YASL_Table_Item item = {k, v};
	inc_ref(&item.value);
	inc_ref(&item.key);
	return item;
}

void del_item(struct YASL_Table_Item *const item) {
	dec_ref(&item->key);
	dec_ref(&item->value);
}

struct YASL_Table *table_new_sized(const size_t base_size) {
	struct YASL_Table *table = (struct YASL_Table *)malloc(sizeof(struct YASL_Table));
	table->base_size = base_size;
	table->size = next_prime(table->base_size);
	table->count = 0;
	table->items = (struct YASL_Table_Item *)calloc((size_t) table->size, sizeof(struct YASL_Table_Item));
	return table;
}

struct YASL_Table *YASL_Table_new(void) {
	return table_new_sized(TABLE_BASESIZE);
}

void YASL_Table_del(struct YASL_Table *const table) {
	if (!table) return;
	DEL_TABLE(table);
	free(table);
}

struct RC_UserData *rcht_new_sized(const size_t base_size) {
        struct RC_UserData *ht = (struct RC_UserData *)malloc(sizeof(struct RC_UserData));
        ht->data = table_new_sized(base_size);
        ht->rc = NEW_RC();
        ht->tag = TABLE_NAME;
        ht->destructor = rcht_del_data;
        ht->mt = NULL;
        return ht;
}

struct RC_UserData *rcht_new(void) {
	return rcht_new_sized(TABLE_BASESIZE);
}

void rcht_del(struct RC_UserData *const hashtable) {
	YASL_Table_del((struct YASL_Table *) hashtable->data);
	free(hashtable);
}

void rcht_del_data(void *hashtable) {
	YASL_Table_del((struct YASL_Table *) hashtable);
}

static void table_resize(struct YASL_Table *const table, const size_t base_size) {
	if (base_size < TABLE_BASESIZE) return;
	struct YASL_Table *new_table = table_new_sized(base_size);
	FOR_TABLE(i, item, table) {
			YASL_Table_insert_fast(new_table, item->key, item->value);
	}
	table->base_size = new_table->base_size;
	table->count = new_table->count;

	const size_t tmp_size = table->size;
	table->size = new_table->size;
	new_table->size = tmp_size;

	struct YASL_Table_Item *tmp_items = table->items;
	table->items = new_table->items;
	new_table->items = tmp_items;

	YASL_Table_del(new_table);
}

static void table_resize_up(struct YASL_Table *const table) {
	const size_t new_size = table->base_size * 2;
	table_resize(table, new_size);
}

static void table_resize_down(struct YASL_Table *const table) {
	const size_t new_size = table->base_size / 2;
	table_resize(table, new_size);
}

bool isequal_typed(const struct YASL_Object *const a, const struct YASL_Object *const b) {
	return a->type == b->type && isequal(a, b);
}

size_t YASL_Table_getindex(struct YASL_Table *const table, const struct YASL_Object key) {
	size_t index = get_hash(key, table->size, 0);
	struct YASL_Table_Item curr_item = table->items[index];
	size_t i = 1;
	while (curr_item.value.type != Y_UNDEF) {
		if (curr_item.key.type != Y_END) {
			if (isequal_typed(&curr_item.key, &key)) {
				return index;
			}
		}
		index = get_hash(key, table->size, i++);
		curr_item = table->items[index];
	}
	return index;
}

void YASL_Table_insert_fast(struct YASL_Table *const table, const struct YASL_Object key, const struct YASL_Object value) {
	YASL_ASSERT(ishashable(&key), "`key` must be hashable");

	const size_t load = table->count * 100 / table->size;
	if (load > 70) table_resize_up(table);
	struct YASL_Table_Item item = new_item(key, value);
	const size_t index = YASL_Table_getindex(table, key);
	struct YASL_Table_Item curr_item = table->items[index];
	if (isequal_typed(&curr_item.key, &key)) {
		del_item(&curr_item);
	} else {
		table->count++;
	}
	
	table->items[index] = item;
}

bool YASL_Table_insert(struct YASL_Table *const table, const struct YASL_Object key, const struct YASL_Object value) {
	if (!ishashable(&key)) {
		return false;
	}
	YASL_Table_insert_fast(table, key, value);
	return true;
}

void YASL_Table_insert_string_int(struct YASL_Table *const table, const char *const key, const size_t key_len,
				  const int64_t val) {
	struct YASL_String *string = YASL_String_new_sized_heap(0, key_len, copy_char_buffer(key_len, key));
	struct YASL_Object ko = YASL_STR(string);
	struct YASL_Object vo = YASL_INT(val);
	YASL_Table_insert_fast(table, ko, vo);
}

yasl_int YASL_Table_length(const struct YASL_Table *const ht) {
	return (yasl_int)ht->count;
}

struct YASL_Object YASL_Table_search(const struct YASL_Table *const table, const struct YASL_Object key) {
	YASL_ASSERT(table != NULL, "table to search should not be NULL");
	if (!ishashable(&key)) return YASL_END();
	size_t index = get_hash(key, table->size, 0);
	struct YASL_Table_Item item = table->items[index];
	int i = 1;
	while (!obj_isundef(&item.key)) {
		if ((isequal_typed(&item.key, &key))) {
			return item.value;
		}
		index = get_hash(key, table->size, i++);
		item = table->items[index];
	}
	return YASL_END();
}

struct YASL_Object YASL_Table_search_string_int(const struct YASL_Table *const table, const char *const key,
						const size_t key_len) {
	struct YASL_String *string = YASL_String_new_sized_heap(0, key_len, copy_char_buffer(key_len, key));
	struct YASL_Object object = YASL_STR(string);

	struct YASL_Object result = YASL_Table_search(table, object);
	str_del(string);
	return result;
}

void YASL_Table_rm(struct YASL_Table *const table, const struct YASL_Object key) {
	if (!ishashable(&key)) return;
	const size_t load = table->count * 100 / table->size;
	if (load < 10) table_resize_down(table);
	size_t index = get_hash(key, table->size, 0);
	struct YASL_Table_Item item = table->items[index];
	size_t i = 1;
	while (!obj_isundef(&item.key)) {
		if (item.key.type != Y_END) {
			if ((isequal_typed(&item.key, &key))) {
				del_item(&item);
				table->items[index] = TOMBSTONE;
			}
		}
		index = get_hash(key, table->size, i++);
		item = table->items[index];
	}
	table->count--;
}
