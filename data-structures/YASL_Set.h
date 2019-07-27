#include "interpreter/YASL_Object.h"

#define FOR_SET(i, item, table) struct YASL_Object *item; for (size_t i = 0; i < (table)->size; i++) \
                                                  if (item = &table->items[i], item->type != Y_END && !YASL_ISUNDEF(*item))

struct Set {
	size_t size;
	size_t base_size;
	size_t count;
	struct YASL_Object *items;
};

struct Set *set_new(void);
void set_del(void *s);
void set_insert(struct Set *const set, struct YASL_Object value);
struct YASL_Object set_search(const struct Set *const table, const struct YASL_Object key);
void set_rm(struct Set *const table, struct YASL_Object key);

struct Set *set_union(const struct Set *const left, const struct Set *const right);
struct Set *set_intersection(const struct Set *const left, const struct Set *const right);
struct Set *set_symmetric_difference(const struct Set *const left, const struct Set *const right);
struct Set *set_difference(const struct Set *const left, const struct Set *const right);
size_t set_length(const struct Set *const set);
