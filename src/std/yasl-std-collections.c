#include "yasl-std-collections.h"

#include "data-structures/YASL_Set.h"
#include "yasl_state.h"
#include "yasl_aux.h"

// what to prepend to method names in messages to user
#define SET_PRE "collections.set"

static const char *const SET_NAME = "collections.set";

static struct YASL_Set *YASLX_checknset(struct YASL_State *S, const char *name, unsigned n) {
	return (struct YASL_Set *)YASLX_checknuserdata(S, SET_NAME, name, n);
}

static int YASL_collections_set_fromlist(struct YASL_State *S) {
	struct YASL_Set *set = YASL_Set_new();

	YASL_duptop(S);
	YASL_len(S);
	yasl_int len = YASL_popint(S);

	for (yasl_int i = 0; i < len; i++) {
		YASL_listget(S, i);
		if (!YASL_Set_insert(set, vm_peek((struct VM *) S))) {
			YASL_Set_del(S, set);
			vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.",
					  YASL_peektypename(S));
			YASLX_throw_err_type(S);
		}
		YASL_pop(S);
	}

	YASL_pushuserdata(S, set, SET_NAME, YASL_Set_del);
	YASL_loadmt(S, SET_NAME);
	YASL_setmt(S);
	return 1;
}

static int YASL_collections_set_new(struct YASL_State *S) {
	yasl_int i = YASL_peekvargscount(S);
	if (i == 1 && YASL_isnlist(S, 1)) {  // TODO: Fix hack, we are off by one because of the number of VA ARGS.
		return YASL_collections_set_fromlist(S);
	}

	struct YASL_Set *set = YASL_Set_new();
	while (i-- > 0) {
		if (!YASL_Set_insert(set, vm_peek((struct VM *) S))) {
			YASL_Set_del(S, set);
			vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.",
					  YASL_peektypename(S));
			YASLX_throw_err_type(S);
		}
		YASL_pop(S);
	}

	YASL_pushuserdata(S, set, SET_NAME, YASL_Set_del);
	YASL_loadmt(S, SET_NAME);
	YASL_setmt(S);
	return 1;
}

static int YASL_collections_list_new(struct YASL_State *S) {
	yasl_int i = YASL_peekvargscount(S);
	struct RC_UserData *list = rcls_new(&S->vm);
	while (i-- > 0) {
		YASL_List_push((struct YASL_List *) list->data, vm_pop((struct VM *) S));
	}
	YASL_reverse((struct YASL_List *) list->data);
	vm_pushlist((struct VM *)S, list);
	return 1;
}

static int YASL_collections_table_new(struct YASL_State *S) {
	yasl_int i = YASL_peekvargscount(S);
	// If we have an odd number of args, we just pop the last one to balance it out.
	if (i % 2 != 0) {
		YASL_pop(S);
		i--;
	}
	struct RC_UserData *table = rcht_new(&S->vm);
	while (i > 0) {
		struct YASL_Object value = vm_pop((struct VM *)S);
		struct YASL_Object key = vm_pop((struct VM *)S);
		if (!YASL_Table_insert((struct YASL_Table *) table->data, key, value)) {
			rcht_del(table);
			struct YASL_Object mt = YASL_TABLE(S->vm.builtins_htable[Y_TABLE]);
			vm_dec_ref((struct VM *)S, &mt);
			vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.",
					  obj_typename(&key));
			YASLX_throw_err_type(S);
		}
		i -= 2;
	}
	vm_pushtable((struct VM *)S, table);
	return 1;
}

static int YASL_collections_set_tostr(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".tostr", 0);
	struct YASL_Object *format = vm_peek_p((struct VM *)S);

	if (YASL_Set_length(set) == 0) {
		YASL_pushlit(S, "set()");
		return 1;
	}

	YASL_ByteBuffer bb = NEW_BB(8);
	YASL_ByteBuffer_extend(&bb, (const byte *)"set(", strlen("set("));

	FOR_SET(i, item, set) {
		vm_push((struct VM *)S, *item);
		vm_stringify_top_format((struct VM *) S, format);
		struct YASL_String *str = vm_popstr((struct VM *) S);

		YASL_ByteBuffer_extend(&bb, (const byte *)YASL_String_chars(str), YASL_String_len(str));
		YASL_ByteBuffer_extend(&bb, (const byte *)", ", 2);
	}

	bb.count -= 2;
	YASL_ByteBuffer_add_byte(&bb, ')');
	vm_pushstr_bb((struct VM *)S, &bb);
	return 1;
}

static int YASL_collections_set_tolist(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".tolist", 0);
	struct RC_UserData *list = rcls_new(&S->vm);
	struct YASL_List *ls = (struct YASL_List *)list->data;
	FOR_SET(i, item, set) {
		YASL_List_push(ls, *item);
	}

	vm_pushlist(&S->vm, list);
	return 1;
}

#define YASL_COLLECTIONS_SET_BINOP(name, fn) \
static int YASL_collections_set_##name(struct YASL_State *S) {\
	struct YASL_Set *right = YASLX_checknset(S, SET_PRE "." #name, 1);\
	struct YASL_Set *left = YASLX_checknset(S, SET_PRE "." #name, 0);\
\
	struct YASL_Set *tmp = fn(left, right);\
\
	YASL_pushuserdata(S, tmp, SET_NAME, YASL_Set_del);\
	YASL_loadmt(S, SET_NAME);\
	YASL_setmt(S);\
	return 1;\
}

YASL_COLLECTIONS_SET_BINOP(__band, YASL_Set_intersection)
YASL_COLLECTIONS_SET_BINOP(__bor, YASL_Set_union)
YASL_COLLECTIONS_SET_BINOP(__bxor, YASL_Set_symmetric_difference)

YASL_COLLECTIONS_SET_BINOP(__bandnot, YASL_Set_difference)

static int YASL_collections_set___eq(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checknset(S, SET_PRE ".__eq", 1);
	struct YASL_Set *left = YASLX_checknset(S, SET_PRE ".__eq", 0);

	if (YASL_Set_length(left) != YASL_Set_length(right)) {
		YASL_pushbool(S, false);
		return 1;
	}

	FOR_SET(i, item, left) {
		if (!YASL_Set_search(right, *item)) {
			YASL_pushbool(S, false);
			return 1;
		}
	}

	YASL_pushbool(S, true);
	return 1;
}

static int YASL_collections_set___gt(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checknset(S, SET_PRE ".__gt", 1);
	struct YASL_Set *left = YASLX_checknset(S, SET_PRE ".__gt", 0);

	FOR_SET(i, item, right) {
		if (!YASL_Set_search(left, *item)) {
			YASL_pushbool(S, false);
			return 1;
		}
	}

	YASL_pushbool(S, YASL_Set_length(left) > YASL_Set_length(right));
	return 1;
}

static int YASL_collections_set___ge(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checknset(S, SET_PRE ".__ge", 1);
	struct YASL_Set *left = YASLX_checknset(S, SET_PRE ".__ge", 0);

	FOR_SET(i, item, right) {
		if (!YASL_Set_search(left, *item)) {
			YASL_pushbool(S, false);
			return 1;
		}
	}

	YASL_pushbool(S, YASL_Set_length(left) >= YASL_Set_length(right));
	return 1;
}

static int YASL_collections_set___lt(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checknset(S, SET_PRE ".__lt", 1);
	struct YASL_Set *left = YASLX_checknset(S, SET_PRE ".__lt", 0);

	FOR_SET(i, item, left) {
		if (!YASL_Set_search(right, *item)) {
			YASL_pushbool(S, false);
			return 1;
		}
	}

	YASL_pushbool(S, YASL_Set_length(left) < YASL_Set_length(right));
	return 1;
}

static int YASL_collections_set___le(struct YASL_State *S) {
	struct YASL_Set *right = YASLX_checknset(S, SET_PRE ".__le", 1);
	struct YASL_Set *left = YASLX_checknset(S, SET_PRE ".__le", 0);

	FOR_SET(i, item, left) {
		if (!YASL_Set_search(right, *item)) {
			YASL_pushbool(S, false);
			return 1;
		}
	}

	YASL_pushbool(S, YASL_Set_length(left) <= YASL_Set_length(right));
	return 1;
}

static int YASL_collections_set___len(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".__len", 0);

	YASL_pushint(S, YASL_Set_length(set));
	return 1;
}

static int YASL_collections_set_add(struct YASL_State *S) {
	struct YASL_Object val = vm_pop(&S->vm);

	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".add", 0);

	if (!YASL_Set_insert(set, val)) {
		vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.", obj_typename(&val));
		YASLX_throw_err_type(S);
	}
	return 1;
}

static int YASL_collections_set_remove(struct YASL_State *S) {
	struct YASL_Object val =  vm_pop((struct VM *)S);

	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".remove", 0);

	YASL_Set_rm(set, val);

	vm_push((struct VM *)S, val);
	return 1;
}

static int YASL_collections_set_copy(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".copy", 0);

	struct YASL_Set *tmp = YASL_Set_new();
	FOR_SET(i, item, set) {
		YASL_Set_insert(tmp, *item);
	}

	YASL_pushuserdata(S, tmp, SET_NAME, YASL_Set_del);
	YASL_loadmt(S, SET_NAME);
	YASL_setmt(S);
	return 1;
}

static int YASL_collections_set_clear(struct YASL_State *S) {
	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".clear", 0);

	FOR_SET(i, item, set) {
		YASL_Set_rm(set, *item);
	}
	return 1;
}

static int YASL_collections_set_next(struct YASL_State *S) {
	struct YASL_Object *curr = vm_peek_p(&S->vm);
	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".__next", 0);

	size_t index = obj_isundef(curr) ? 0 : YASL_Set_getindex(set, *curr) + 1;

	while (set->size > index &&
	       (set->items[index].type == Y_END || set->items[index].type == Y_UNDEF)) {
		index++;
	}

	if (set->size <= index) {
		YASL_pushbool(S, false);
		return 1;
	} else {
		vm_push(&S->vm, set->items[index]);
		vm_push(&S->vm, set->items[index]);
		YASL_pushbool(S, true);
		return 3;
	}
}

static int YASL_collections_set___iter(struct YASL_State *S) {
	YASL_pushcfunction(S, YASL_collections_set_next, 2);
	YASL_pushundef(S);
	return 2;
}

static int YASL_collections_set___get(struct YASL_State *S) {
	struct YASL_Object object = vm_pop(&S->vm);
	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".__get", 0);

	YASL_pushbool(S, YASL_Set_search(set, object));
	return 1;
}

static int YASL_collections_set_has(struct YASL_State *S) {
	struct YASL_Object object = vm_pop(&S->vm);
	struct YASL_Set *set = YASLX_checknset(S, SET_PRE ".has", 0);

	YASL_pushbool(S, YASL_Set_search(set, object));
	return 1;
}

int YASL_decllib_collections(struct YASL_State *S) {
	YASL_pushtable(S);
	YASL_registermt(S, SET_NAME);

	struct YASLX_function functions[] = {
		{ "tostr", &YASL_collections_set_tostr, 2 },
		{ "tolist", &YASL_collections_set_tolist, 1 },
		{ "__band", &YASL_collections_set___band, 2 },
		{ "__bor", &YASL_collections_set___bor, 2 },
		{ "__bxor", &YASL_collections_set___bxor, 2 },
		{ "__bandnot", &YASL_collections_set___bandnot, 2 },
		{ "__len", &YASL_collections_set___len, 1 },
		{ "__lt", &YASL_collections_set___lt, 2 },
		{ "__le", &YASL_collections_set___le, 2 },
		{ "__gt", &YASL_collections_set___gt, 2 },
		{ "__ge", &YASL_collections_set___ge, 2 },
		{ "__eq", &YASL_collections_set___eq, 2 },
		{ "add", &YASL_collections_set_add, 2 },
		{ "remove", &YASL_collections_set_remove, 2 },
		{ "copy", &YASL_collections_set_copy, 1 },
		{ "clear", &YASL_collections_set_clear, 1 },
		{ "has", &YASL_collections_set_has, 2 },
		{ "__iter", &YASL_collections_set___iter, 1 },
		{ "__get", &YASL_collections_set___get, 2 },
		{ NULL, NULL, 0 }
	};

	YASL_loadmt(S, SET_NAME);
	YASLX_tablesetfunctions(S, functions);
	YASL_pop(S);

	YASL_pushtable(S);
	YASLX_initglobal(S, "collections");

	YASL_loadglobal(S, "collections");

	struct YASLX_function constructors[] = {
		{ "set", &YASL_collections_set_new, -1 },
		{ "list", &YASL_collections_list_new, -1 },
		{ "table", &YASL_collections_table_new, -1 },
		{ NULL, NULL, 0}
	};

	YASLX_tablesetfunctions(S, constructors);
	YASL_pop(S);

	return YASL_SUCCESS;
}
