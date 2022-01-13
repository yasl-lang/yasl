#include "YASL_Set.h"

#include "util/hash_function.h"
#include "util/prime.h"

#define SET_BASESIZE 30

static struct YASL_Set *set_new_sized(const size_t base_size) {
	struct YASL_Set *set = (struct YASL_Set *)malloc(sizeof(struct YASL_Set));
	set->base_size = base_size;
	set->size = next_prime(set->base_size);
	set->count = 0;
	set->items = (struct YASL_Object *)calloc((size_t) set->size, sizeof(struct YASL_Object));
	return set;
}

struct YASL_Set *YASL_Set_new(void) {
	return set_new_sized(SET_BASESIZE);
}

void YASL_Set_del(void *s) {
	if (!s) return;
	struct YASL_Set *set = (struct YASL_Set *)s;
	FOR_SET(i, item, set) {
		dec_ref(item);
	}
	free(set->items);
	free(set);
}

static void set_resize(struct YASL_Set *const set, const size_t base_size) {
	if (base_size < SET_BASESIZE) return;
	struct YASL_Set *new_set = set_new_sized(base_size);
	FOR_SET(i, item, set) {
			YASL_Set_insert(new_set, *item);
	}
	set->base_size = new_set->base_size;
	set->count = new_set->count;

	const size_t tmp_size = set->size;
	set->size = new_set->size;
	new_set->size = tmp_size;

	struct YASL_Object *tmp_items = set->items;
	set->items = new_set->items;
	new_set->items = tmp_items;

	YASL_Set_del(new_set);
}

static void set_resize_up(struct YASL_Set *set) {
	const size_t new_size = set->base_size * 2;
	set_resize(set, new_size);
}

static void set_resize_down(struct YASL_Set *set) {
	const size_t new_size = set->base_size / 2;
	set_resize(set, new_size);
}

bool YASL_Set_insert(struct YASL_Set *const set, struct YASL_Object value) {
	if (!ishashable(&value)) {
		return false;
	}

	const size_t load = set->count * 100 / set->size;
	if (load > 70) set_resize_up(set);
	size_t index = YASL_Set_getindex(set, value);
	struct YASL_Object curr_item = set->items[index];
	if (isequal_typed(&curr_item, &value)) {
		dec_ref(&curr_item);
	} else {
		set->count++;
	}

	inc_ref(&value);
	set->items[index] = value;
	return true;
}

bool YASL_Set_search(const struct YASL_Set *const set, const struct YASL_Object key) {
	size_t index = get_hash(key, set->size, 0);
	struct YASL_Object item = set->items[index];
	size_t i = 1;
	while (!obj_isundef(&item)) {
		if ((isequal(&item, &key))) {
			return true;
		}
		index = get_hash(key, set->size, i++);
		item = set->items[index];
	}
	return false;
}

void YASL_Set_rm(struct YASL_Set *const set, struct YASL_Object key) {
	if (!ishashable(&key)) {
		return;
	}
	const size_t load = set->count * 100 / set->size;
	if (load < 10) set_resize_down(set);
	size_t index = get_hash(key, set->size, 0);
	struct YASL_Object item = set->items[index];
	size_t i = 1;
	while (!obj_isundef(&item)) {
		if (item.type != Y_END) {
			if ((isequal(&item, &key))) {
				dec_ref(&item);
				set->items[index] = YASL_END();
			}
		}
		index = get_hash(key, set->size, i++);
		item = set->items[index];
	}
	set->count--;
}

size_t YASL_Set_getindex(const struct YASL_Set *const set, const struct YASL_Object value) {
	size_t index = get_hash(value, set->size, 0);
	struct YASL_Object curr_item = set->items[index];
	size_t i = 1;
	while (!obj_isundef(&curr_item)) {
		if (curr_item.type != Y_END) {
			if ((isequal_typed(&curr_item, &value))) {
				return index;
			}
		}
		index = get_hash(value, set->size, i++);
		curr_item = set->items[index];
	}
	return index;
}

struct YASL_Set *YASL_Set_union(const struct YASL_Set *const left, const struct YASL_Set *const right) {
	struct YASL_Set *tmp = YASL_Set_new();
	FOR_SET(i, iteml, left) {
			YASL_Set_insert(tmp, *iteml);
	}
	FOR_SET(i, itemr, right) {
			YASL_Set_insert(tmp, *itemr);
	}
	return tmp;
}

struct YASL_Set *YASL_Set_intersection(const struct YASL_Set *const left, const struct YASL_Set *const right) {
	struct YASL_Set *tmp = YASL_Set_new();
	FOR_SET(i, iteml, left) {
		bool cond = YASL_Set_search(right, *iteml);
		if (cond) {
			YASL_Set_insert(tmp, *iteml);
		}
	}
	return tmp;
}

struct YASL_Set *YASL_Set_symmetric_difference(const struct YASL_Set *const left, const struct YASL_Set *const right) {
	struct YASL_Set *tmp = YASL_Set_new();
	FOR_SET(i, iteml, left) {
		bool cond = YASL_Set_search(right, *iteml);
		if (!cond) {
			YASL_Set_insert(tmp, *iteml);
		}
	}
	FOR_SET(i, itemr, right) {
		bool cond = YASL_Set_search(left, *itemr);
		if (!cond) {
			YASL_Set_insert(tmp, *itemr);
		}
	}

	return tmp;
}

struct YASL_Set *YASL_Set_difference(const struct YASL_Set *const left, const struct YASL_Set *const right) {
	struct YASL_Set *tmp = YASL_Set_new();
	FOR_SET(i, iteml, left) {
		bool cond = YASL_Set_search(right, *iteml);
		if (!cond) {
			YASL_Set_insert(tmp, *iteml);
		}
	}
	return tmp;
}

size_t YASL_Set_length(const struct YASL_Set *const set) {
	return set->count;
}
