#include "table_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_error.h"
#include "yasl_state.h"


int table___get(struct YASL_State *S) {
	struct YASL_Object key = vm_pop((struct VM *) S);
	if (!YASL_istable(S)) {
		YASL_print_err_bad_arg_type(S, "table.__get", 0, "table", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Table *ht = YASL_GETTABLE(vm_peek((struct VM *) S));
	struct YASL_Object result = YASL_Table_search(ht, key);
	if (result.type == Y_END) {
		S->vm.sp++;  // TODO: fix this
		//vm_push((struct VM *)S, key);
		return -1;
	} else {
		vm_pop((struct VM *) S);
		vm_push((struct VM *) S, result);
	}
	return YASL_SUCCESS;
}

int table___set(struct YASL_State *S) {
	struct YASL_Object val = vm_pop((struct VM *) S);
	struct YASL_Object key = vm_pop((struct VM *) S);
	if (!YASL_istable(S)) {
		YASL_print_err_bad_arg_type(S, "table.__set", 0, "table", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Table *ht = YASL_GETTABLE(vm_pop((struct VM *) S));
	if (YASL_ISUNDEF(val)) {
		YASL_Table_rm(ht, key);
		return YASL_SUCCESS;
	}

	if (!YASL_Table_insert(ht, key, val)) {
		vm_print_err_type(&S->vm, "unable to use mutable object of type %s as key.\n", YASL_TYPE_NAMES[key.type]);
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int object_tostr(struct YASL_State *S) {
	enum YASL_Types index = vm_peek((struct VM *) S, S->vm.sp).type;
	struct YASL_Object key = YASL_STR(YASL_String_new_sized(strlen("tostr"), "tostr"));
	struct YASL_Object result = YASL_Table_search(S->vm.builtins_htable[index], key);
	str_del(YASL_GETSTR(key));
	YASL_GETCFN(result)->value(S);
	return YASL_SUCCESS;
}

int list_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count);

int table_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count) {
	size_t string_count = 0;
	size_t string_size = 8;
	char *string = (char *) malloc(string_size);

	string[string_count++] = '{';
	struct YASL_Table *table = vm_peektable((struct VM *) S, S->vm.sp);
	if (table->count == 0) {
		vm_pop((struct VM *) S);
		string[string_count++] = '}';
		vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized_heap(0, string_count, string)));
		return YASL_SUCCESS;
	}

	FOR_TABLE(i, item, table) {
		vm_push((struct VM *) S, item->key);

		object_tostr(S);

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

		string[string_count++] = ':';
		string[string_count++] = ' ';

		vm_push((struct VM *) S, item->value);

		if (YASL_ISLIST(vm_peek((struct VM *) S, S->vm.sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist((struct VM *) S, S->vm.sp)) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (string_count + strlen("[...], ") >= string_size) {
					string_size *= 2;
					string = (char *) realloc(string, string_size);
				}
				memcpy(string + string_count, "[...], ", strlen("[...], "));
				string_count += strlen("[...], ");
				vm_pop((struct VM *) S);
				continue;
			} else {
				size_t tmp_buffer_size =
					buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = (void **) malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_count] = vm_peeklist((struct VM *) S, S->vm.sp);
				list_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_count + 1);
				free(tmp_buffer);
			}
		} else if (YASL_ISTABLE(vm_peek((struct VM *) S, S->vm.sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist((struct VM *) S, S->vm.sp)) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (string_count + strlen("{...}, ") >= string_size) {
					string_size *= 2;
					string = (char *) realloc(string, string_size);
				}
				memcpy(string + string_count, "{...}, ", strlen("{...}, "));
				string_count += strlen("{...}, ");
				vm_pop((struct VM *) S);
				continue;
			} else {
				size_t tmp_buffer_size =
					buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = (void **) malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_count] = vm_peeklist((struct VM *) S, S->vm.sp);
				table_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
				free(tmp_buffer);
			}
		} else {
			vm_stringify_top((struct VM *) S);
		}

		str = vm_popstr((struct VM *) S);
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

	vm_pop((struct VM *) S);

	if (string_count + 2 >= string_size) {
		string_size *= 2;
		string = (char *) realloc(string, string_size);
	}

	string_count -= 2;
	string[string_count++] = '}';

	vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized_heap(0, string_count, string)));

	return YASL_SUCCESS;
}

int table_tostr(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		YASL_print_err_bad_arg_type(S, "table.tostr", 0, "table", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}

	void **buffer = (void **) malloc(8 * sizeof(void *));
	buffer[0] = vm_peektable((struct VM *) S, S->vm.sp);
	table_tostr_helper(S, buffer, 8, 1);
	free(buffer);

	return YASL_SUCCESS;
}

int table_keys(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		YASL_print_err_bad_arg_type(S, "table.keys", 0, "table", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Table *ht = YASL_GETTABLE(vm_pop((struct VM *) S));
	struct RC_UserData *ls = rcls_new();
	FOR_TABLE(i, item, ht) {
			YASL_List_append((struct YASL_List *) ls->data, (item->key));
	}

	vm_push((struct VM *) S, YASL_LIST(ls));
	return YASL_SUCCESS;
}

int table_values(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		YASL_print_err_bad_arg_type(S, "table.values", 0, "table", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Table *ht = YASL_GETTABLE(vm_pop((struct VM *) S));
	struct RC_UserData *ls = rcls_new();
	FOR_TABLE(i, item, ht) {
			YASL_List_append((struct YASL_List *) ls->data, (item->value));
	}
	vm_push((struct VM *) S, YASL_LIST(ls));
	return YASL_SUCCESS;
}

int table_remove(struct YASL_State *S) {
	struct YASL_Object key = vm_pop((struct VM *) S);
	if (!YASL_istable(S)) {
		YASL_print_err_bad_arg_type(S, "table.remove", 0, "table", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Table *ht = YASL_GETTABLE(vm_peek((struct VM *) S));

	YASL_Table_rm(ht, key);
	return YASL_SUCCESS;
}

int table_clone(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		YASL_print_err_bad_arg_type(S, "table.copy", 0, "table", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Table *ht = YASL_GETTABLE(vm_pop((struct VM *) S));
	struct RC_UserData *new_ht = rcht_new_sized(ht->base_size);

	FOR_TABLE(i, item, ht) {
			YASL_Table_insert_fast((struct YASL_Table *) new_ht->data, item->key, item->value);
	}

	vm_push((struct VM *) S, YASL_TABLE(new_ht));
	return YASL_SUCCESS;
}

int table_clear(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		YASL_print_err_bad_arg_type(S, "table.clear", 0, "table", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_Table *ht = YASL_GETTABLE(vm_peek((struct VM *) S));
	inc_ref(&vm_peek((struct VM *) S));
	FOR_TABLE(i, item, ht) {
		del_item(item);
	}

	ht->count = 0;
	ht->size = TABLE_BASESIZE;
	free(ht->items);
	ht->items = (struct YASL_Table_Item *) calloc((size_t) ht->size, sizeof(struct YASL_Table_Item));
	dec_ref(&vm_peek((struct VM *) S));
	vm_pop((struct VM *) S);
	vm_pushundef((struct VM *) S);
	return YASL_SUCCESS;
}
