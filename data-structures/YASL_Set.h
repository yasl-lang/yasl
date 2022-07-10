#ifndef YASL_YASL_SET_H_
#define YASL_YASL_SET_H_

#include "interpreter/YASL_Object.h"

#define FOR_SET(i, item, table) struct YASL_Object *item; for (size_t i = 0; i < (table)->size; i++) \
                                                  if (item = &table->items[i], item->type != Y_END && !obj_isundef(item))

struct YASL_Set {
	size_t size;
	size_t base_size;
	size_t count;
	struct YASL_Object *items;
};

struct YASL_Set *YASL_Set_new(void);
void YASL_Set_del(struct YASL_State *S, void *set);
bool YASL_Set_insert(struct YASL_Set *const set, struct YASL_Object value) /* YASL_WARN_UNUSED */;
bool YASL_Set_search(const struct YASL_Set *const set, const struct YASL_Object key);
void YASL_Set_rm(struct YASL_Set *const set, struct YASL_Object key);
size_t YASL_Set_getindex(const struct YASL_Set *const set, const struct YASL_Object value);

struct YASL_Set *YASL_Set_union(const struct YASL_Set *const left, const struct YASL_Set *const right);
struct YASL_Set *YASL_Set_intersection(const struct YASL_Set *const left, const struct YASL_Set *const right);
struct YASL_Set *YASL_Set_symmetric_difference(const struct YASL_Set *const left, const struct YASL_Set *const right);
struct YASL_Set *YASL_Set_difference(const struct YASL_Set *const left, const struct YASL_Set *const right);
size_t YASL_Set_length(const struct YASL_Set *const set);

#endif
