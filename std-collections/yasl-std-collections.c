#include "yasl-std-collections.h"

#include "data-structures/YASL_Set.h"
#include "interpreter/VM.h"

static struct YASL_Table *set_mt = NULL;

static int YASL_collections_set_new(struct YASL_State *S) {
	struct YASL_Set *set = YASL_Set_new();
	yasl_int i = vm_popint((struct VM *)S);
	while (i-- > 0) {
		YASL_Set_insert(set, vm_pop((struct VM *) S));
	}
	struct YASL_Object *obj = YASL_UserData(set, T_SET, set_mt, YASL_Set_del);
	vm_push((struct VM *)S, *obj);
	free(obj);
	return YASL_SUCCESS;
}

static int YASL_collections_list_new(struct YASL_State *S) {
	yasl_int i = vm_popint((struct VM *)S);
	struct RC_UserData *list = rcls_new();
	while (i-- > 0) {
		YASL_List_append((struct YASL_List *) list->data, vm_pop((struct VM *) S));
	}
	YASL_reverse((struct YASL_List *) list->data);
	vm_pushlist((struct VM *)S, list);
	return YASL_SUCCESS;
}

static int YASL_collections_table_new(struct YASL_State *S) {
	yasl_int i = vm_popint((struct VM *)S);
	// TODO: error for odd num args?
	if (i % 2 != 0) {
		vm_pushundef((struct VM *)S);
	}
	struct RC_UserData *table = rcht_new();
	while (i > 0) {
		struct YASL_Object value = vm_pop((struct VM *)S);
		struct YASL_Object key = vm_pop((struct VM *)S);
		YASL_Table_insert((struct YASL_Table *)table->data, key, value);
		i -= 2;
	}
	vm_pushtable((struct VM *)S, table);
	return YASL_SUCCESS;
}

static int YASL_collections_set_tostr(struct YASL_State *S) {
	if (!YASL_top_isuserdata(S, T_SET)) {
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_top_popuserdata(S);

	size_t string_count = 0;
	size_t string_size = 8;
	char *string = (char *)malloc(string_size);
	string_count += strlen("set(");
	memcpy(string, "set(", strlen("set("));
	if (YASL_Set_length(set) == 0) {
		string[string_count++] = ')';
		YASL_pushstring(S, string, string_count);
		return YASL_SUCCESS;
	}
	FOR_SET(i, item, set) {
		vm_push((struct VM *)S, *item);
		vm_stringify_top((struct VM *) S);
		struct YASL_String *str = vm_popstr((struct VM *) S);
		while (string_count + YASL_String_len(str) >= string_size) {
			string_size *= 2;
			string = (char *) realloc(string, string_size);
		}

		memcpy(string + string_count, str->str + str->start, YASL_String_len(str));
		string_count += YASL_String_len(str);

		if (string_count + 2 >= string_size) {
			string_size *= 2;
			string = (char *) realloc(string, string_size);
		}

		string[string_count++] = ',';
		string[string_count++] = ' ';
	}

	string_count -= 2;
	string[string_count++] = ')';
	vm_pushstr((struct VM *)S, YASL_String_new_sized_heap(0, string_count, string));
	return YASL_SUCCESS;
}

#define YASL_COLLECTIONS_SET_BINOP(name, fn) \
static int YASL_collections_set_##name(struct YASL_State *S) {\
	if (!YASL_top_isuserdata(S, T_SET)) {\
		return YASL_TYPE_ERROR;\
	}\
	struct YASL_Set *right = (struct YASL_Set *)YASL_top_popuserdata(S);\
\
	if (!YASL_top_isuserdata(S, T_SET)) {\
		return YASL_TYPE_ERROR;\
	}\
	struct YASL_Set *left = (struct YASL_Set *)YASL_top_popuserdata(S);\
\
	struct YASL_Set *tmp = fn(left, right);\
\
	YASL_pushuserdata(S, tmp, T_SET, set_mt, YASL_Set_del);\
	return YASL_SUCCESS;\
}

YASL_COLLECTIONS_SET_BINOP(__band, YASL_Set_intersection)
YASL_COLLECTIONS_SET_BINOP(__bor, YASL_Set_union)
YASL_COLLECTIONS_SET_BINOP(__bxor, YASL_Set_symmetric_difference)
YASL_COLLECTIONS_SET_BINOP(__sub, YASL_Set_difference)

static int YASL_collections_set___len(struct YASL_State *S) {
	if (!YASL_top_isuserdata(S, T_SET)) {
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_top_popuserdata(S);

	YASL_pushinteger(S, YASL_Set_length(set));
	return YASL_SUCCESS;
}

static int YASL_collections_set_add(struct YASL_State *S) {
	struct YASL_Object *right_obj = YASL_popobject(S);


	if (YASL_ISLIST(*right_obj) || YASL_ISTABLE(*right_obj) || YASL_ISUSERDATA(*right_obj)) {
		printf("Error: unable to insert mutable object of type %x into set.\n", right_obj->type);
		return YASL_TYPE_ERROR;
	}
	if (!YASL_top_isuserdata(S, T_SET)) {
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *left = (struct YASL_Set *)YASL_top_popuserdata(S);

	YASL_Set_insert(left, *right_obj);

	return YASL_SUCCESS;
}

static int YASL_collections_set_remove(struct YASL_State *S) {
	struct YASL_Object right_obj =  vm_pop((struct VM *)S);


	if (YASL_ISLIST(right_obj) || YASL_ISTABLE(right_obj) || YASL_ISUSERDATA(right_obj)) {
		printf("Error: unable to remove mutable object of type %x into set.\n", right_obj.type);
		return YASL_TYPE_ERROR;
	}
	if (!YASL_top_isuserdata(S, T_SET)) {
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *left = (struct YASL_Set *)YASL_top_popuserdata(S);

	YASL_Set_rm(left, right_obj);

	vm_push((struct VM *)S, right_obj);
	return YASL_SUCCESS;
}

static int YASL_collections_set_copy(struct YASL_State *S) {
	if (!YASL_top_isuserdata(S, T_SET)) {
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_top_popuserdata(S);

	struct YASL_Set *tmp = YASL_Set_new();
	FOR_SET(i, item, set) {
			YASL_Set_insert(tmp, *item);
	}

	YASL_pushuserdata(S, tmp, T_SET, set_mt, YASL_Set_del);
	return YASL_SUCCESS;
}

static int YASL_collections_set_clear(struct YASL_State *S) {
	if (!YASL_top_isuserdata(S, T_SET)) {
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_top_popuserdata(S);

	FOR_SET(i, item, set) {
			YASL_Set_rm(set, *item);
	}

	return YASL_SUCCESS;
}

static int YASL_collections_set_contains(struct YASL_State *S) {
	struct YASL_Object *object = YASL_popobject(S);
	if (!YASL_top_isuserdata(S, T_SET)) {
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_top_popuserdata(S);

	vm_push((struct VM *)S, YASL_Set_search(set, *object));
	return YASL_SUCCESS;
}

int YASL_load_collections(struct YASL_State *S) {
	if (!set_mt) {
		set_mt = YASL_Table_new();
		YASL_Table_insert_literalcstring_cfunction(set_mt, "tostr", YASL_collections_set_tostr, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__band", YASL_collections_set___band, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__bor", YASL_collections_set___bor, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__bxor", YASL_collections_set___bxor, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__sub", YASL_collections_set___sub, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__len", YASL_collections_set___len, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "add", YASL_collections_set_add, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "remove", YASL_collections_set_remove, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "copy", YASL_collections_set_copy, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "clear", YASL_collections_set_clear, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "contains", YASL_collections_set_contains, 2);
	}

	struct YASL_Object *collections = YASL_Table();
	YASL_declglobal(S, "collections");
	YASL_pushobject(S, collections);

	YASL_pushobject(S, collections);
	YASL_pushlitszstring(S, "set");
	YASL_pushcfunction(S, YASL_collections_set_new, -1);
	YASL_settable(S);

	YASL_pushobject(S, collections);
	YASL_pushlitszstring(S, "list");
	YASL_pushcfunction(S, YASL_collections_list_new, -1);
	YASL_settable(S);

	YASL_pushobject(S, collections);
	YASL_pushlitszstring(S, "table");
	YASL_pushcfunction(S, YASL_collections_table_new, -1);
	YASL_settable(S);

	YASL_setglobal(S, "collections");

	free(collections);
	return YASL_SUCCESS;
}

