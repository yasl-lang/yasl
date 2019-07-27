#pragma once

#include "data-structures/YASL_ByteBuffer.h"
#include "interpreter/refcount.h"
#include "interpreter/userdata.h"

#define FOR_LIST(i, name, list) struct YASL_Object name; for (size_t i = 0; i < (list)->count && (name = (list)->items[i], 1); i++)

struct YASL_List {
	size_t size;
	size_t count;
	struct YASL_Object *items;
};

struct RC_UserData *ls_new(void);
struct YASL_List *list_new_sized(const size_t base_size);
struct RC_UserData* ls_new_sized(const size_t base_size);
void ls_del_data(void *ls);
void ls_insert(struct YASL_List *const ls, const int64_t index, struct YASL_Object value);
void ls_append(struct YASL_List *const ls, struct YASL_Object value);
struct YASL_Object ls_search(const struct YASL_List *const ls, const int64_t index);
void ls_reverse(struct YASL_List *const ls);
