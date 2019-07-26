#include "YASL_list.h"

#include "interpreter/YASL_Object.h"
#include "data-structures/YASL_hashtable.h"

int isvalueinarray(int64_t val, int64_t *arr, int size) {
	int i;
	for (i = 0; i < size; i++) {
		if (arr[i] == val)
			return 1;
	}
	return 0;
}

struct List *list_new_sized(const size_t base_size) {
	struct List *list = (struct List *)malloc(sizeof(struct List));
	list->size = base_size;
	list->count = 0;
	list->items = (struct YASL_Object *)malloc(sizeof(struct YASL_Object) * list->size);
	return list;
}

struct RC_UserData* ls_new_sized(const size_t base_size) {
	struct RC_UserData *ls = (struct RC_UserData *)malloc(sizeof(struct RC_UserData));

	ls->data = list_new_sized(base_size);
	ls->rc = rc_new();
	ls->destructor = ls_del_data;
	ls->tag = T_LIST;
	return ls;
}

struct RC_UserData* ls_new(void) {
	return ls_new_sized(LS_BASESIZE);
}

void ls_del_data(void *ls) {
	for (size_t i = 0; i < ((struct List *) ls)->count; i++) dec_ref(((struct List *) ls)->items + i);
	free(((struct List *) ls)->items);
	free(ls);
}

void ls_del(struct RC_UserData *ls) {
	for (size_t i = 0; i < ((struct List *) ls->data)->count; i++) dec_ref(((struct List *) ls->data)->items + i);
	free(((struct List *) ls->data)->items);
	free(((struct List *) ls->data));
	rc_del(ls->rc);
	free(ls);
}

static void ls_resize(struct List* ls, const size_t base_size) {
	ls->items = (struct YASL_Object *)realloc(ls->items, base_size * sizeof(struct YASL_Object));
	ls->size = base_size;
}

static void ls_resize_up(struct List* ls) {
	const size_t new_size = ls->size ? ls->size * 2 : 1;
	ls_resize(ls, new_size);
}

/*
static void ls_resize_down(struct RC_List* ls) {
    const int new_size = ((struct List *)ls->data)->size / 2;
    ls_resize(ls, new_size);
}
*/

void ls_insert(struct List *const ls, const int64_t index, struct YASL_Object value) {
	if (ls->count >= ls->size) ls_resize_up(ls);
	dec_ref(ls->items + index);
	ls->items[index] = value;
	ls->count++;
	inc_ref(&value);
}

void ls_append(struct List* ls, struct YASL_Object value) {
	if (ls->count >= ls->size) ls_resize_up(ls);
	ls->items[ls->count++] = value;
	inc_ref(&value);
}

struct YASL_Object ls_search(struct List* ls, int64_t index) {
	struct YASL_Object undobj = UNDEF_C;
	if (index < -(int64_t) ls->count || index >= (int64_t) ls->count) return undobj;
	else if (0 <= index) return ls->items[index];
	else return ls->items[ls->count + index];
}

void ls_reverse(struct List *ls) {
	for (size_t i = 0; i < ls->count / 2; i++) {
		struct YASL_Object tmp = ls->items[i];
		ls->items[i] = ls->items[ls->count - i - 1];
		ls->items[ls->count - i - 1] = tmp;
	}
}
