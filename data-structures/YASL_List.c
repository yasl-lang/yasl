#include "YASL_List.h"

#include "interpreter/YASL_Object.h"
#include "data-structures/YASL_HashTable.h"

struct YASL_List *list_new_sized(const size_t base_size) {
	struct YASL_List *list = (struct YASL_List *)malloc(sizeof(struct YASL_List));
	list->size = base_size;
	list->count = 0;
	list->items = (struct YASL_Object *)malloc(sizeof(struct YASL_Object) * list->size);
	return list;
}

struct RC_UserData* rcls_new_sized(const size_t base_size) {
	struct RC_UserData *ls = (struct RC_UserData *)malloc(sizeof(struct RC_UserData));

	ls->data = list_new_sized(base_size);
	ls->rc = rc_new();
	ls->destructor = ls_del_data;
	ls->tag = T_LIST;
	return ls;
}

struct RC_UserData* rcls_new(void) {
	return rcls_new_sized(LIST_BASESIZE);
}

void ls_del_data(void *ls) {
	for (size_t i = 0; i < ((struct YASL_List *) ls)->count; i++) dec_ref(((struct YASL_List *) ls)->items + i);
	free(((struct YASL_List *) ls)->items);
	free(ls);
}

void rcls_del(struct RC_UserData *const ls) {
	for (size_t i = 0; i < ((struct YASL_List *) ls->data)->count; i++) dec_ref(((struct YASL_List *) ls->data)->items + i);
	free(((struct YASL_List *) ls->data)->items);
	free(((struct YASL_List *) ls->data));
	rc_del(ls->rc);
	free(ls);
}

static void ls_resize(struct YASL_List *const ls, const size_t base_size) {
	ls->items = (struct YASL_Object *)realloc(ls->items, base_size * sizeof(struct YASL_Object));
	ls->size = base_size;
}

static void ls_resize_up(struct YASL_List *const ls) {
	const size_t new_size = ls->size ? ls->size * 2 : 1;
	ls_resize(ls, new_size);
}

/*
static void ls_resize_down(struct RC_List* ls) {
    const int new_size = ((struct YASL_List *)ls->data)->size / 2;
    ls_resize(ls, new_size);
}
*/

void ls_insert(struct YASL_List *const ls, const int64_t index, struct YASL_Object value) {
	if (ls->count >= ls->size) ls_resize_up(ls);
	dec_ref(ls->items + index);
	ls->items[index] = value;
	ls->count++;
	inc_ref(&value);
}

void ls_append(struct YASL_List *const ls, struct YASL_Object value) {
	if (ls->count >= ls->size) ls_resize_up(ls);
	ls->items[ls->count++] = value;
	inc_ref(&value);
}

struct YASL_Object ls_search(const struct YASL_List *const ls, const int64_t index) {
	struct YASL_Object undobj = UNDEF_C;
	if (index < -(int64_t) ls->count || index >= (int64_t) ls->count) return undobj;
	else if (0 <= index) return ls->items[index];
	else return ls->items[ls->count + index];
}

void ls_reverse(struct YASL_List *const ls) {
	for (size_t i = 0; i < ls->count / 2; i++) {
		struct YASL_Object tmp = ls->items[i];
		ls->items[i] = ls->items[ls->count - i - 1];
		ls->items[ls->count - i - 1] = tmp;
	}
}
