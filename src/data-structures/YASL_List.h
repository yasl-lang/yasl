#ifndef YASL_YASL_LIST_H_
#define YASL_YASL_LIST_H_

#include "data-structures/YASL_ByteBuffer.h"
#include "interpreter/refcount.h"
#include "interpreter/userdata.h"

#define LIST_BASESIZE 4

#define FOR_LIST_START(i, name, list, start) struct YASL_Object name; for (size_t i = start; i < (list)->count && (name = (list)->items[i], 1); i++)
#define FOR_LIST(i, name, list) FOR_LIST_START(i, name, list, 0)

struct YASL_List {
	size_t size;
	size_t count;
	struct YASL_Object *items;
};

struct YASL_List *YASL_List_new_sized(const size_t base_size);
void YASL_List_del_data(struct YASL_State *S, void *ls);
size_t YASL_List_len(const struct YASL_List *const ls);
void YASL_List_push(struct YASL_List *const ls, struct YASL_Object value);
void YASL_List_insert(struct YASL_List *const ls, size_t index, struct YASL_Object value);
void YASL_reverse(struct YASL_List *const ls);

struct RC_UserData *rcls_new(struct VM *vm);
struct RC_UserData* rcls_new_sized(struct VM *vm, const size_t base_size);

#endif
