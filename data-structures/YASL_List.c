#include "YASL_List.h"

#include "interpreter/YASL_Object.h"
#include "data-structures/YASL_Table.h"

#include <string.h>

const char *const LIST_NAME = "list";

struct YASL_List *YASL_List_new_sized(const size_t base_size) {
	struct YASL_List *list = (struct YASL_List *)malloc(sizeof(struct YASL_List));
	list->size = base_size;
	list->count = 0;
	list->items = (struct YASL_Object *)malloc(sizeof(struct YASL_Object) * list->size);
	return list;
}

struct RC_UserData* rcls_new_sized(const size_t base_size) {
	struct RC_UserData *ls = (struct RC_UserData *)malloc(sizeof(struct RC_UserData));

	ls->data = YASL_List_new_sized(base_size);
	ls->rc = NEW_RC();
	ls->mt = NULL;
	ls->destructor = YASL_List_del_data;
	ls->tag = LIST_NAME;
	return ls;
}

struct RC_UserData* rcls_new(void) {
	return rcls_new_sized(LIST_BASESIZE);
}

void YASL_List_del_data(void *ls) {
	for (size_t i = 0; i < ((struct YASL_List *) ls)->count; i++) dec_ref(((struct YASL_List *) ls)->items + i);
	free(((struct YASL_List *) ls)->items);
	free(ls);
}

yasl_int YASL_List_length(const struct YASL_List *const ls) {
	return (yasl_int)ls->count;
}

static void ls_resize(struct YASL_List *const ls, const size_t base_size) {
	ls->items = (struct YASL_Object *)realloc(ls->items, base_size * sizeof(struct YASL_Object));
	ls->size = base_size;
}

static void ls_resize_up(struct YASL_List *const ls) {
	const size_t new_size = ls->size ? ls->size * 2 : 1;
	ls_resize(ls, new_size);
}

void YASL_List_append(struct YASL_List *const ls, struct YASL_Object value) {
	if (ls->count >= ls->size) ls_resize_up(ls);
	ls->items[ls->count++] = value;
	inc_ref(&value);
}

void YASL_List_insert(struct YASL_List *const ls, size_t index, struct YASL_Object value) {
	if (ls->count >= ls->size) ls_resize_up(ls);
	memmove(ls->items + index + 1, ls->items + index, (ls->count - index) * sizeof(struct YASL_Object));
	ls->items[index] = value;
	ls->count++;
	inc_ref(&value);
}

void YASL_reverse(struct YASL_List *const ls) {
	for (size_t i = 0; i < ls->count / 2; i++) {
		struct YASL_Object tmp = ls->items[i];
		ls->items[i] = ls->items[ls->count - i - 1];
		ls->items[ls->count - i - 1] = tmp;
	}
}
