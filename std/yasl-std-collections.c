#include "yasl-std-collections.h"

#include "data-structures/YASL_Set.h"
#include "yasl_state.h"

// what to prepend to method names in messages to user
#define SET_PRE "collections.set"

static struct YASL_Set *YASLX_checkset(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isuserdata(S, T_SET)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type set, got arg of type %s.",
				  name, pos, YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return (struct YASL_Set *)YASL_popuserdata(S);
}

static void YASL_collections_set_new(struct YASL_State *S) {
	struct YASL_Set *set = YASL_Set_new();
	yasl_int i = YASL_popint(S);
	while (i-- > 0) {
		if (!YASL_Set_insert(set, vm_peek((struct VM *) S))) {
			vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.",
					  YASL_peektypestr(S));
			YASL_throw_err(S, YASL_TYPE_ERROR);
		}
		YASL_pop(S);
	}

	YASL_pushuserdata(S, set, T_SET, YASL_Set_del);
	YASL_loadmt(S, SET_PRE);
	YASL_setmt(S);
}

static void YASL_collections_list_new(struct YASL_State *S) {
	yasl_int i = YASL_popint(S);
	struct RC_UserData *list = rcls_new();
	ud_setmt(list, S->vm.builtins_htable[Y_LIST]);
	while (i-- > 0) {
		YASL_List_append((struct YASL_List *) list->data, vm_pop((struct VM *) S));
	}
	YASL_reverse((struct YASL_List *) list->data);
	vm_pushlist((struct VM *)S, list);
}

static void YASL_collections_table_new(struct YASL_State *S) {
	yasl_int i = YASL_popint(S);
	// If we have an odd number of args, we just add an undef to balance it out.
	if (i % 2 != 0) {
		YASL_pushundef(S);
	}
	struct RC_UserData *table = rcht_new();
	ud_setmt(table, S->vm.builtins_htable[Y_TABLE]);
	while (i > 0) {
		struct YASL_Object value = vm_pop((struct VM *)S);
		struct YASL_Object key = vm_pop((struct VM *)S);
		if (!YASL_Table_insert((struct YASL_Table *) table->data, key, value)) {
			vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.",
					  YASL_TYPE_NAMES[key.type]);
			YASL_throw_err(S, YASL_TYPE_ERROR);
		}
		i -= 2;
	}
	vm_pushtable((struct VM *)S, table);
}

static void YASL_collections_set_tostr(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checkset(S, SET_PRE ".tostr", 0);

	size_t string_count = 0;
	size_t string_size = 8;
	char *string = (char *)malloc(string_size);
	string_count += strlen("set(");
	memcpy(string, "set(", strlen("set("));
	if (YASL_Set_length(set) == 0) {
		string[string_count++] = ')';
		return YASL_pushstring(S, string, string_count);
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
}

static void YASL_collections_set_tolist(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checkset(S, SET_PRE ".tolist", 0);
	struct RC_UserData *list = rcls_new();
	struct YASL_List *ls = (struct YASL_List *)list->data;
	FOR_SET(i, item, set) {
		YASL_List_append(ls, *item);
	}

	vm_pushlist(&S->vm, list);
}

#define YASL_COLLECTIONS_SET_BINOP(name, fn) \
static void YASL_collections_set_##name(struct YASL_State *S) {\
	struct YASL_Set *right = YASLX_checkset(S, SET_PRE "." #name, 1);\
	struct YASL_Set *left = YASLX_checkset(S, SET_PRE "." #name, 0);\
\
	struct YASL_Set *tmp = fn(left, right);\
\
	YASL_pushuserdata(S, tmp, T_SET, YASL_Set_del);\
	YASL_loadmt(S, SET_PRE);\
	YASL_setmt(S);\
}

YASL_COLLECTIONS_SET_BINOP(__band, YASL_Set_intersection)
YASL_COLLECTIONS_SET_BINOP(__bor, YASL_Set_union)
YASL_COLLECTIONS_SET_BINOP(__bxor, YASL_Set_symmetric_difference)

YASL_COLLECTIONS_SET_BINOP(__bandnot, YASL_Set_difference)

static void YASL_collections_set___eq(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checkset(S, SET_PRE ".__eq", 1);
	struct YASL_Set *left = YASLX_checkset(S, SET_PRE ".__eq", 0);

	if (YASL_Set_length(left) != YASL_Set_length(right)) {
		return YASL_pushbool(S, false);
	}

	FOR_SET(i, item, left) {
		if (!YASL_Set_search(right, *item)) {
			return YASL_pushbool(S, false);
		}
	}

	return YASL_pushbool(S, true);
}

static void YASL_collections_set___gt(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checkset(S, SET_PRE ".__gt", 1);
	struct YASL_Set *left = YASLX_checkset(S, SET_PRE ".__gt", 0);

	FOR_SET(i, item, right) {
		if (!YASL_Set_search(left, *item)) {
			return YASL_pushbool(S, false);
		}
	}

	return YASL_pushbool(S, YASL_Set_length(left) > YASL_Set_length(right));
}

static void YASL_collections_set___ge(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checkset(S, SET_PRE ".__ge", 1);
	struct YASL_Set *left = YASLX_checkset(S, SET_PRE ".__ge", 0);

	FOR_SET(i, item, right) {
		if (!YASL_Set_search(left, *item)) {
			return YASL_pushbool(S, false);
		}
	}

	return YASL_pushbool(S, YASL_Set_length(left) >= YASL_Set_length(right));
}

static void YASL_collections_set___lt(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checkset(S, SET_PRE ".__lt", 1);
	struct YASL_Set *left = YASLX_checkset(S, SET_PRE ".__lt", 0);

	FOR_SET(i, item, left) {
		if (!YASL_Set_search(right, *item)) {
			return YASL_pushbool(S, false);
		}
	}

	return YASL_pushbool(S, YASL_Set_length(left) < YASL_Set_length(right));
}

static void YASL_collections_set___le(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checkset(S, SET_PRE ".__le", 1);
	struct YASL_Set *left = YASLX_checkset(S, SET_PRE ".__le", 0);

	FOR_SET(i, item, left) {
		if (!YASL_Set_search(right, *item)) {
			return YASL_pushbool(S, false);
		}
	}

	return YASL_pushbool(S, YASL_Set_length(left) <= YASL_Set_length(right));
}

static void YASL_collections_set___len(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checkset(S, SET_PRE ".__len", 0);

	return YASL_pushint(S, YASL_Set_length(set));
}

static void YASL_collections_set_add(struct YASL_State *S) {
	struct YASL_Object val = vm_pop(&S->vm);

	struct YASL_Set *set = YASLX_checkset(S, SET_PRE ".add", 0);

	if (!YASL_Set_insert(set, val)) {
		vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.", YASL_TYPE_NAMES[val.type]);
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
}

static void YASL_collections_set_remove(struct YASL_State *S) {
	struct YASL_Object val =  vm_pop((struct VM *)S);

	struct YASL_Set *set = YASLX_checkset(S, SET_PRE ".remove", 0);

	YASL_Set_rm(set, val);

	vm_push((struct VM *)S, val);
}

static void YASL_collections_set_copy(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checkset(S, SET_PRE ".copy", 0);

	struct YASL_Set *tmp = YASL_Set_new();
	FOR_SET(i, item, set) {
		YASL_Set_insert(tmp, *item);
	}

	YASL_pushuserdata(S, tmp, T_SET, YASL_Set_del);
	YASL_loadmt(S, SET_PRE);
	YASL_setmt(S);
}

static void YASL_collections_set_clear(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checkset(S, SET_PRE ".clear", 0);

	FOR_SET(i, item, set) {
		YASL_Set_rm(set, *item);
	}
}

static void YASL_collections_set___get(struct YASL_State *S) {
	struct YASL_Object object = vm_pop(&S->vm);
	struct YASL_Set *set = YASLX_checkset(S, SET_PRE ".__get", 0);

	return YASL_pushbool(S, YASL_Set_search(set, object));
}

int YASL_decllib_collections(struct YASL_State *S) {
	YASL_pushtable(S);
	YASL_registermt(S, SET_PRE);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "tostr");
	YASL_pushcfunction(S, YASL_collections_set_tostr, 1);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "tolist");
	YASL_pushcfunction(S, YASL_collections_set_tolist, 1);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__band");
	YASL_pushcfunction(S, YASL_collections_set___band, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__bor");
	YASL_pushcfunction(S, YASL_collections_set___bor, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__bxor");
	YASL_pushcfunction(S, YASL_collections_set___bxor, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__bandnot");
	YASL_pushcfunction(S, YASL_collections_set___bandnot, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__len");
	YASL_pushcfunction(S, YASL_collections_set___len, 1);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__eq");
	YASL_pushcfunction(S, YASL_collections_set___eq, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__gt");
	YASL_pushcfunction(S, YASL_collections_set___gt, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__ge");
	YASL_pushcfunction(S, YASL_collections_set___ge, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__lt");
	YASL_pushcfunction(S, YASL_collections_set___lt, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__le");
	YASL_pushcfunction(S, YASL_collections_set___le, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "add");
	YASL_pushcfunction(S, YASL_collections_set_add, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "remove");
	YASL_pushcfunction(S, YASL_collections_set_remove, 2);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "copy");
	YASL_pushcfunction(S, YASL_collections_set_copy, 1);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "clear");
	YASL_pushcfunction(S, YASL_collections_set_clear, 1);
	YASL_tableset(S);

	YASL_loadmt(S, SET_PRE);
	YASL_pushlitszstring(S, "__get");
	YASL_pushcfunction(S, YASL_collections_set___get, 2);
	YASL_tableset(S);


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

