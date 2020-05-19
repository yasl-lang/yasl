#include "yasl-std-collections.h"

#include "data-structures/YASL_Set.h"
#include "yasl_state.h"

// what to prepend to method names in messages to user
#define SET_PRE "collections.set"

static struct YASL_Table *set_mt = NULL;

static int YASL_collections_set_new(struct YASL_State *S) {
	struct YASL_Set *set = YASL_Set_new();
	yasl_int i = YASL_popint(S);
	while (i-- > 0) {
		if (!YASL_Set_insert(set, vm_peek((struct VM *) S))) {
			vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.\n",
					  YASL_TYPE_NAMES[vm_peek((struct VM*)S).type]);
			return YASL_TYPE_ERROR;
		}
		YASL_pop(S);
	}
	YASL_pushuserdata(S, set, T_SET, set_mt, YASL_Set_del);
	return YASL_SUCCESS;
}

static int YASL_collections_list_new(struct YASL_State *S) {
	yasl_int i = YASL_popint(S);
	struct RC_UserData *list = rcls_new();
	while (i-- > 0) {
		YASL_List_append((struct YASL_List *) list->data, vm_pop((struct VM *) S));
	}
	YASL_reverse((struct YASL_List *) list->data);
	vm_pushlist((struct VM *)S, list);
	return YASL_SUCCESS;
}

static int YASL_collections_table_new(struct YASL_State *S) {
	yasl_int i = YASL_popint(S);
	// If we have an odd number of args, we just add an undef to balance it out.
	if (i % 2 != 0) {
		YASL_pushundef(S);
	}
	struct RC_UserData *table = rcht_new();
	while (i > 0) {
		struct YASL_Object value = vm_pop((struct VM *)S);
		struct YASL_Object key = vm_pop((struct VM *)S);
		if (!YASL_Table_insert((struct YASL_Table *) table->data, key, value)) {
			vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.\n",
					  YASL_TYPE_NAMES[key.type]);
			return YASL_TYPE_ERROR;
		}
		i -= 2;
	}
	vm_pushtable((struct VM *)S, table);
	return YASL_SUCCESS;
}

static int YASL_collections_set_tostr(struct YASL_State *S) {
	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",
				  SET_PRE ".tostr", 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_popuserdata(S);

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

static int YASL_collections_set_tolist(struct YASL_State *S) {
	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",
				  SET_PRE ".tolist", 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_popuserdata(S);
	struct RC_UserData *list = rcls_new();
	struct YASL_List *ls = (struct YASL_List *)list->data;
	FOR_SET(i, item, set) {
		YASL_List_append(ls, *item);
	}

	vm_pushlist(&S->vm, list);

	return YASL_SUCCESS;
}

#define YASL_COLLECTIONS_SET_BINOP(name, fn) \
static int YASL_collections_set_##name(struct YASL_State *S) {\
	if (!YASL_isuserdata(S, T_SET)) {\
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",\
				SET_PRE "." #name, 1, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);\
		return YASL_TYPE_ERROR;\
	}\
	struct YASL_Set *right = (struct YASL_Set *)YASL_popuserdata(S);\
\
	if (!YASL_isuserdata(S, T_SET)) {\
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",\
				SET_PRE "." #name, 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);\
		return YASL_TYPE_ERROR;\
	}\
	struct YASL_Set *left = (struct YASL_Set *)YASL_popuserdata(S);\
\
	struct YASL_Set *tmp = fn(left, right);\
\
	YASL_pushuserdata(S, tmp, T_SET, set_mt, YASL_Set_del);\
	return YASL_SUCCESS;\
}

YASL_COLLECTIONS_SET_BINOP(__band, YASL_Set_intersection)
YASL_COLLECTIONS_SET_BINOP(__bor, YASL_Set_union)
YASL_COLLECTIONS_SET_BINOP(__bxor, YASL_Set_symmetric_difference)

YASL_COLLECTIONS_SET_BINOP(__bandnot, YASL_Set_difference)

static int YASL_collections_set___len(struct YASL_State *S) {
	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",
				  SET_PRE ".__len", 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_popuserdata(S);

	YASL_pushint(S, YASL_Set_length(set));
	return YASL_SUCCESS;
}

static int YASL_collections_set_add(struct YASL_State *S) {
	struct YASL_Object val = vm_pop(&S->vm);

	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",
				  SET_PRE ".add", 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_popuserdata(S);

	if (!YASL_Set_insert(set, val)) {
		vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.\n", YASL_TYPE_NAMES[val.type]);
		return YASL_TYPE_ERROR;
	}

	return YASL_SUCCESS;
}

static int YASL_collections_set_remove(struct YASL_State *S) {
	struct YASL_Object right_obj =  vm_pop((struct VM *)S);

	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",
				  SET_PRE ".remove", 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *left = (struct YASL_Set *)YASL_popuserdata(S);

	YASL_Set_rm(left, right_obj);

	vm_push((struct VM *)S, right_obj);
	return YASL_SUCCESS;
}

static int YASL_collections_set_copy(struct YASL_State *S) {
	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",
				  SET_PRE ".copy", 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_popuserdata(S);

	struct YASL_Set *tmp = YASL_Set_new();
	FOR_SET(i, item, set) {
			YASL_Set_insert(tmp, *item);
	}

	YASL_pushuserdata(S, tmp, T_SET, set_mt, YASL_Set_del);
	return YASL_SUCCESS;
}

static int YASL_collections_set_clear(struct YASL_State *S) {
	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",
				  SET_PRE ".clear", 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_popuserdata(S);

	FOR_SET(i, item, set) {
			YASL_Set_rm(set, *item);
	}

	return YASL_SUCCESS;
}

static int YASL_collections_set_contains(struct YASL_State *S) {
	struct YASL_Object object = vm_pop(&S->vm);
	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.\n",
				  SET_PRE ".contains", 0, YASL_TYPE_NAMES[vm_peek(&S->vm).type]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Set *set = (struct YASL_Set *)YASL_popuserdata(S);

	vm_push((struct VM *)S, YASL_Set_search(set, object));
	return YASL_SUCCESS;
}

int YASL_load_collections(struct YASL_State *S) {
	if (!set_mt) {
		set_mt = YASL_Table_new();
		YASL_Table_insert_literalcstring_cfunction(set_mt, "tostr", YASL_collections_set_tostr, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "tolist", YASL_collections_set_tolist, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__band", YASL_collections_set___band, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__bor", YASL_collections_set___bor, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__bxor", YASL_collections_set___bxor, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__bandnot", YASL_collections_set___bandnot, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "__len", YASL_collections_set___len, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "add", YASL_collections_set_add, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "remove", YASL_collections_set_remove, 2);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "copy", YASL_collections_set_copy, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "clear", YASL_collections_set_clear, 1);
		YASL_Table_insert_literalcstring_cfunction(set_mt, "contains", YASL_collections_set_contains, 2);
	}

	YASL_declglobal(S, "collections");
	YASL_pushtable(S);
	YASL_setglobal(S, "collections");

	YASL_loadglobal(S, "collections");
	YASL_pushlitszstring(S, "set");
	YASL_pushcfunction(S, YASL_collections_set_new, -1);
	YASL_tableset(S);

	YASL_loadglobal(S, "collections");
	YASL_pushlitszstring(S, "list");
	YASL_pushcfunction(S, YASL_collections_list_new, -1);
	YASL_tableset(S);

	YASL_loadglobal(S, "collections");
	YASL_pushlitszstring(S, "table");
	YASL_pushcfunction(S, YASL_collections_table_new, -1);
	YASL_tableset(S);

	return YASL_SUCCESS;
}

