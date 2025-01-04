#include "table_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_error.h"
#include "yasl_state.h"

static struct YASL_Table *YASLX_checkntable(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isntable(S, pos)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, pos, YASL_TABLE_NAME);
	}
	return (struct YASL_Table *)YASL_peeknuserdata(S, pos);
}

int table___bor(struct YASL_State *S) {
	struct YASL_Table *left = YASLX_checkntable(S, "table.__bor", 0);
	struct YASL_Table *right = YASLX_checkntable(S, "table.__bor", 1);

	struct RC_UserData *new_ht = rcht_new_sized(&S->vm, left->base_size);

	FOR_TABLE(i, litem, left) {
		YASL_Table_insert_fast((struct YASL_Table *) new_ht->data, litem->key, litem->value);
	}

	FOR_TABLE(i, ritem, right) {
		YASL_Table_insert_fast((struct YASL_Table *) new_ht->data, ritem->key, ritem->value);
	}

	vm_push((struct VM *) S, YASL_TABLE(new_ht));
	return 1;
}

int table___eq(struct YASL_State *S) {
	struct YASL_Table *left = YASLX_checkntable(S, "table.__eq", 0);
	struct YASL_Table *right = YASLX_checkntable(S, "table.__eq", 1);

	if (left->count != right->count) {
		YASL_pushbool(S, false);
		return 1;
	}

	FOR_TABLE(i, item, left) {
		struct YASL_Object search = YASL_Table_search(right, item->key);
		if (search.type == Y_END) {
			YASL_pushbool(S, false);
			return 1;
		}
		vm_push((struct VM *) S, item->value);
		vm_push((struct VM *) S, search);
		vm_EQ((struct VM *) S);
		if (!YASL_popbool(S)) {
			YASL_pushbool(S, false);
			return 1;
		}
	}

	YASL_pushbool(S, true);
	return 1;
}

int table___get(struct YASL_State *S) {
	struct YASL_Object key = vm_pop((struct VM *) S);
	struct YASL_Table *ht = YASLX_checkntable(S, "table.__get", 0);
	struct YASL_Object result = YASL_Table_search(ht, key);
	if (result.type == Y_END) {
		vm_pushundef(&S->vm);
	} else {
		vm_push((struct VM *) S, result);
	}
	return 1;
}

int table___len(struct YASL_State *S) {
	struct YASL_Table *ht = YASLX_checkntable(S, "table.__len", 0);
	YASL_pushint(S, YASL_Table_length(ht));
	return 1;
}

static int table___next(struct YASL_State *S) {
	YASLX_checkntable(S, "table.__next", 0);
	struct YASL_Object key = vm_pop(&S->vm);
	struct YASL_Table *table = vm_peektable(&S->vm);

	size_t index = obj_isundef(&key) ? 0 : YASL_Table_getindex(table, key) + 1;

	while (table->size > index &&
	       (table->items[index].key.type == Y_END || table->items[index].key.type == Y_UNDEF)) {
		index++;
	}

	if (table->size <= index) {
		YASL_pushbool(S, false);
		return 1;
	}

	vm_push(&S->vm, table->items[index].key);
	vm_push(&S->vm, table->items[index].key);
	YASL_pushbool(S, true);
	return 3;
}

int table___iter(struct YASL_State *S) {
	YASLX_checkntable(S, "table.__iter", 0);
	YASL_pushcfunction(S, &table___next, 2);
	YASL_pushundef(S);
	return 2;
}

int table___set(struct YASL_State *S) {
	struct YASL_Object val = vm_pop((struct VM *) S);
	struct YASL_Object key = vm_pop((struct VM *) S);
	struct YASL_Table *ht = YASLX_checkntable(S, "table.__set", 0);
	if (obj_isundef(&val)) {
		YASL_Table_rm(ht, key);
		return 1;
	}

	if (!YASL_Table_insert(ht, key, val)) {
		vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.", obj_typename(&key));
		YASLX_throw_err_type(S);
	}
	return 1;
}

void list_tostr_helper(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format);

#define FOUND_LIST "[...], "
#define FOUND_TABLE "{...}, "

bool buffer_contains(BUFFER(ptr) buffer, void *val) {
	for (size_t j = 0; j < buffer.count; j++) {
		if (buffer.items[j] == val) {
			return true;
		}
	}
	return false;
}

void rec_call(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format, void (*f)(struct YASL_State *, BUFFER(ptr), struct YASL_Object *)) {
	BUFFER(ptr) tmp = BUFFER_COPY(ptr)(&buffer);
	BUFFER_PUSH(ptr)(&tmp, vm_peeklist((struct VM *)S));

	f(S, tmp, format);

	BUFFER_CLEANUP(ptr)(&tmp);
}

void table_tostr_helper(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format) {
	struct YASL_Table *table = vm_peektable((struct VM *) S);
	if (table->count == 0) {
		YASL_pop(S);
		YASL_pushlit(S, "{}");
		return;
	}

	YASL_ByteBuffer bb = NEW_BB(8);

	YASL_ByteBuffer_add_byte(&bb, '{');

	FOR_TABLE(i, item, table) {
		vm_push((struct VM *) S, item->key);

		vm_stringify_top_format(&S->vm, format);

		struct YASL_String *str = vm_popstr((struct VM *) S);
		YASL_ByteBuffer_extend(&bb, (unsigned char *)YASL_String_chars(str), YASL_String_len(str));
		YASL_ByteBuffer_extend(&bb, (unsigned char *)": ", strlen(": "));

		vm_push((struct VM *) S, item->value);

		if (vm_islist((struct VM *) S)) {
			bool found = buffer_contains(buffer, vm_peeklist((struct VM *) S));
			if (found) {
				YASL_ByteBuffer_extend(&bb, (unsigned char *)FOUND_LIST, strlen(FOUND_LIST));
				vm_pop((struct VM *) S);
				continue;
			} else {
				rec_call(S, buffer, format, &list_tostr_helper);
			}
		} else if (vm_istable((struct VM *) S, S->vm.sp)) {
			bool found = buffer_contains(buffer, vm_peeklist((struct VM *) S));
			if (found) {
				YASL_ByteBuffer_extend(&bb, (unsigned char *)FOUND_TABLE, strlen(FOUND_TABLE));
				vm_pop((struct VM *) S);
				continue;
			} else {
				rec_call(S, buffer, format, &table_tostr_helper);
			}
		} else {
			vm_stringify_top_format((struct VM *) S, format);
		}

		str = vm_popstr((struct VM *) S);
		YASL_ByteBuffer_extend(&bb, (unsigned char *)YASL_String_chars(str), YASL_String_len(str));
		YASL_ByteBuffer_extend(&bb, (unsigned char *)", ", strlen(", "));
	}

	vm_pop((struct VM *) S);

	bb.count -= 2;
	YASL_ByteBuffer_add_byte(&bb, '}');

	vm_pushstr_bb((struct VM *) S, &bb);
}

int table_tostr(struct YASL_State *S) {
	struct YASL_Table *ht = YASLX_checkntable(S, "table.tostr", 0);
	struct YASL_Object format = vm_pop((struct VM *)S);
	inc_ref(&format);

	BUFFER(ptr) buffer;
	BUFFER_INIT(ptr)(&buffer, 8);
	BUFFER_PUSH(ptr)(&buffer, ht);
	table_tostr_helper(S, buffer, &format);
	BUFFER_CLEANUP(ptr)(&buffer);

	dec_ref(&format);
	return 1;
}

int table_keys(struct YASL_State *S) {
	struct YASL_Table *ht = YASLX_checkntable(S, "table.keys", 0);
	struct RC_UserData *ls = rcls_new(&S->vm);
	FOR_TABLE(i, item, ht) {
		YASL_List_push((struct YASL_List *) ls->data, (item->key));
	}

	vm_pushlist((struct VM *)S, ls);
	return 1;
}

int table_values(struct YASL_State *S) {
	struct YASL_Table *ht = YASLX_checkntable(S, "table.values", 0);
	struct RC_UserData *ls = rcls_new(&S->vm);
	FOR_TABLE(i, item, ht) {
		YASL_List_push((struct YASL_List *) ls->data, (item->value));
	}
	vm_pushlist((struct VM *)S, ls);
	return 1;
}

int table_remove(struct YASL_State *S) {
	struct YASL_Object key = vm_pop((struct VM *) S);
	struct YASL_Table *ht = YASLX_checkntable(S, "table.remove", 0);

	YASL_Table_rm(ht, key);
	return 1;
}

int table_copy(struct YASL_State *S) {
	struct YASL_Table *ht = YASLX_checkntable(S, "table.copy", 0);
	struct RC_UserData *new_ht = rcht_new_sized(&S->vm, ht->base_size);

	FOR_TABLE(i, item, ht) {
		YASL_Table_insert_fast((struct YASL_Table *) new_ht->data, item->key, item->value);
	}

	ud_setmt(&S->vm, new_ht, obj_get_metatable(&S->vm, vm_peek(&S->vm)));
	vm_push((struct VM *) S, YASL_TABLE(new_ht));
	return 1;
}

int table_clear(struct YASL_State *S) {
	struct YASL_Table *ht = YASLX_checkntable(S, "table.clear", 0);

	inc_ref(&vm_peek((struct VM *) S));
	FOR_TABLE(i, item, ht) {
		del_item(item);
	}

	ht->count = 0;
	ht->size = TABLE_BASESIZE;
	free(ht->items);
	ht->items = (struct YASL_Table_Item *) calloc((size_t) ht->size, sizeof(struct YASL_Table_Item));
	vm_dec_ref(&S->vm, &vm_peek((struct VM *) S));

	return 0;
}
