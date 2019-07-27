#include "YASL_Set.h"

#include "data-structures/hash_function.h"

#define SET_BASESIZE 30

struct Set *set_new_sized(const size_t base_size) {
	struct Set *set = (struct Set *)malloc(sizeof(struct Set));
	set->base_size = base_size;
	set->size = next_prime(set->base_size);
	set->count = 0;
	set->items = (struct YASL_Object *)calloc((size_t) set->size, sizeof(struct YASL_Object));
	return set;
}

struct Set *set_new(void) {
	return set_new_sized(SET_BASESIZE);
}

void set_del(void *s) {
	if (!s) return;
	struct Set *set = (struct Set *)s;
	FOR_SET(i, item, set) {
		dec_ref(item);
	}
	free(set->items);
	free(set);
}

static void set_resize(struct Set *const set, const size_t base_size) {
	if (base_size < SET_BASESIZE) return;
	struct Set *new_set = set_new_sized(base_size);
	FOR_SET(i, item, set) {
		set_insert(new_set, *item);
	}
	set->base_size = new_set->base_size;
	set->count = new_set->count;

	const size_t tmp_size = set->size;
	set->size = new_set->size;
	new_set->size = tmp_size;

	struct YASL_Object *tmp_items = set->items;
	set->items = new_set->items;
	new_set->items = tmp_items;

	set_del(new_set);
}

static void set_resize_up(struct Set *set) {
	const size_t new_size = set->base_size * 2;
	set_resize(set, new_size);
}

static void set_resize_down(struct Set *set) {
	const size_t new_size = set->base_size / 2;
	set_resize(set, new_size);
}

void set_insert(struct Set *const set, struct YASL_Object value) {
	const size_t load = set->count * 100 / set->size;
	if (load > 70) set_resize_up(set);
	size_t index = get_hash(value, set->size, 0);
	inc_ref(&value);
	struct YASL_Object curr_item = set->items[index];
	size_t i = 1;
	while (!YASL_ISUNDEF(curr_item)) {
		if (curr_item.type != Y_END) {
			if (!isfalsey(isequal(curr_item, value))) {
				dec_ref(&curr_item);
				set->items[index] = value;
				return;
			}
		}
		index = get_hash(value, set->size, i++);
		curr_item = set->items[index];
	}
	set->items[index] = value;
	set->count++;
}

struct YASL_Object set_search(const struct Set *const table, const struct YASL_Object key) {
	size_t index = get_hash(key, table->size, 0);
	struct YASL_Object item = table->items[index];
	size_t i = 1;
	while (!YASL_ISUNDEF(item)) {
		if (!isfalsey(isequal(item, key))) {
			return YASL_BOOL(true);
		}
		index = get_hash(key, table->size, i++);
		item = table->items[index];
	}
	return YASL_BOOL(false);
}

void set_rm(struct Set *const table, struct YASL_Object key) {
	const size_t load = table->count * 100 / table->size;
	if (load < 10) set_resize_down(table);
	size_t index = get_hash(key, table->size, 0);
	struct YASL_Object item = table->items[index];
	size_t i = 1;
	while (!YASL_ISUNDEF(item)) {
		if (item.type != Y_END) {
			if (!isfalsey(isequal(item, key))) {
				dec_ref(&item);
				table->items[index] = YASL_END();
			}
		}
		index = get_hash(key, table->size, i++);
		item = table->items[index];
	}
	table->count--;
}

struct Set *set_union(const struct Set *const left, const struct Set *const right) {
	struct Set *tmp = set_new();
	FOR_SET(i, iteml, left) {
		set_insert(tmp, *iteml);
	}
	FOR_SET(i, itemr, right) {
		set_insert(tmp, *itemr);
	}
	return tmp;
}

struct Set *set_intersection(const struct Set *const left, const struct Set *const right) {
	struct Set *tmp = set_new();
	FOR_SET(i, iteml, left) {
		struct YASL_Object cond = set_search(right, *iteml);
		if (YASL_GETBOOL(cond)) {
			set_insert(tmp, *iteml);
		}
	}
	return tmp;
}

struct Set *set_symmetric_difference(const struct Set *const left, const struct Set *const right) {
	struct Set *tmp = set_new();
	FOR_SET(i, iteml, left) {
		struct YASL_Object cond = set_search(right, *iteml);
		if (!YASL_GETBOOL(cond)) {
			set_insert(tmp, *iteml);
		}
	}
	FOR_SET(i, itemr, right) {
		struct YASL_Object cond = set_search(left, *itemr);
		if (!YASL_GETBOOL(cond)) {
			set_insert(tmp, *itemr);
		}
	}

	return tmp;
}

struct Set *set_difference(const struct Set *const left, const struct Set *const right) {
	struct Set *tmp = set_new();
	FOR_SET(i, iteml, left) {
		struct YASL_Object cond = set_search(right, *iteml);
		if (!YASL_GETBOOL(cond)) {
			set_insert(tmp, *iteml);
		}
	}
	return tmp;
}

size_t set_length(const struct Set *const set) {
	return set->count;
}
