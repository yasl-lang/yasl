#include "table_methods.h"

#include <stdio.h>

#include "yasl_state.h"
#include "VM.h"
#include "YASL_Object.h"

int table___get(struct YASL_State *S) {
    struct YASL_Object key = vm_pop((struct VM *)S);
    ASSERT_TYPE((struct VM *)S, Y_TABLE, "table.__get");
    struct Table* ht = YASL_GETTABLE(vm_peek((struct VM *)S));
    struct YASL_Object result = table_search(ht, key);
    if (result.type == Y_END) {
        S->vm.sp++;  // TODO: fix this
        //vm_push((struct VM *)S, key);
        return -1;
    }
    else {
        vm_pop((struct VM *)S);
        vm_push((struct VM *)S, result);
    }
    return 0;
}

int table___set(struct YASL_State *S) {
	struct YASL_Object val = vm_pop((struct VM *)S);
	struct YASL_Object key = vm_pop((struct VM *)S);
	ASSERT_TYPE((struct VM *)S, Y_TABLE, "table.__set");
	struct Table *ht = YASL_GETTABLE(vm_pop((struct VM *)S));

	if (YASL_ISLIST(key) || YASL_ISTABLE(key) || YASL_ISUSERDATA(key)) {
		printf("Error: unable to use mutable object of type %x as key.\n", key.type);
		return -1;
	}
	table_insert(ht, key, val);
	vm_push((struct VM *)S, val);
	return 0;
}

int object_tostr(struct YASL_State *S) {
	YASL_Types index = VM_PEEK((struct VM *)S, S->vm.sp).type;
	struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
	struct YASL_Object result = table_search(S->vm.builtins_htable[index], key);
	str_del(YASL_GETSTR(key));
	YASL_GETCFN(result)->value(S);
	return 0;
}

int list_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count);

int table_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count) {
	size_t string_count = 0;
	size_t string_size = 8;
	char *string = malloc(string_size);

	string[string_count++] = '{';
	struct Table *table = vm_peektable((struct VM *)S, S->vm.sp);
	if (table->count == 0) {
		string[string_count++] = '}';
		vm_push((struct VM *)S, YASL_STR(str_new_sized_heap(0, string_count, string)));
		return 0;
	}

	FOR_TABLE(i, item, table) {
		vm_push((struct VM *)S, item->key);

		object_tostr(S);

		String_t *str = vm_popstr((struct VM *)S);
		while (string_count + yasl_string_len(str) >= string_size) {
			string_size *= 2;
			string = realloc(string, string_size);
		}

		memcpy(string + string_count, str->str + str->start, yasl_string_len(str));
		string_count += yasl_string_len(str);

		if (string_count + 2 >= string_size) {
			string_size *= 2;
			string = realloc(string, string_size);
		}

		string[string_count++] = ':';
		string[string_count++] = ' ';

		vm_push((struct VM *)S, item->value);

		if (YASL_ISLIST(VM_PEEK((struct VM *)S, S->vm.sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist((struct VM *)S, S->vm.sp)) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (string_count + strlen("[...], ") >= string_size) {
					string_size *= 2;
					string = realloc(string, string_size);
				}
				memcpy(string + string_count, "[...], ", strlen("[...], "));
				string_count += strlen("[...], ");
				continue;
			} else {
				size_t tmp_buffer_size = buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_count] = vm_peeklist((struct VM *)S, S->vm.sp);
				list_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
				free(tmp_buffer);
			}
		} else if (YASL_ISTABLE(VM_PEEK((struct VM *)S, S->vm.sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist((struct VM *)S, S->vm.sp)) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (string_count + strlen("{...}, ") >= string_size) {
					string_size *= 2;
					string = realloc(string, string_size);
				}
				memcpy(string + string_count, "{...}, ", strlen("{...}, "));
				string_count += strlen("{...}, ");
				continue;
			} else {
				size_t tmp_buffer_size = buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_size] = vm_peeklist((struct VM *)S, S->vm.sp);
				table_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
			}
		} else {
			vm_stringify_top((struct VM *)S);
		}

		str = vm_popstr((struct VM *)S);
		while (string_count + yasl_string_len(str) >= string_size) {
			string_size *= 2;
			string = realloc(string, string_size);
		}

		memcpy(string + string_count, str->str + str->start, yasl_string_len(str));
		string_count += yasl_string_len(str);

		if (string_count + 2 >= string_size) {
			string_size *= 2;
			string = realloc(string, string_size);
		}

		string[string_count++] = ',';
		string[string_count++] = ' ';
	}

	vm_pop((struct VM *)S);

	if (string_count + 2 >= string_size) {
		string_size *= 2;
		string = realloc(string, string_size);
	}

	string_count -= 2;
	string[string_count++] = '}';

	vm_push((struct VM *)S, YASL_STR(str_new_sized_heap(0, string_count, string)));

	return 0;
}

int table_tostr(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_TABLE, "table.tostr");

	void **buffer = malloc(8*sizeof(void *));
	buffer[0] = vm_peektable((struct VM *)S, S->vm.sp);
	table_tostr_helper(S, buffer, 8, 1);
	free(buffer);

	return 0;
}

int table_keys(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_TABLE, "table.keys");
	struct Table *ht = YASL_GETTABLE(vm_pop((struct VM *)S));
	struct RC_UserData *ls = ls_new();
	FOR_TABLE(i, item, ht) {
			ls_append(ls->data, (item->key));
		}

	vm_push((struct VM *)S, YASL_LIST(ls));
	return 0;
}

int table_values(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_TABLE, "table.values");
	struct Table *ht = YASL_GETTABLE(vm_pop((struct VM *)S));
	struct RC_UserData *ls = ls_new();
	FOR_TABLE(i, item, ht) {
		ls_append(ls->data, (item->value));
	}
	vm_push((struct VM *)S, YASL_LIST(ls));
	return 0;
}

int table_clone(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_TABLE, "table.clone");
	struct Table *ht = YASL_GETTABLE(vm_pop((struct VM *)S));
	struct RC_UserData *new_ht = rcht_new_sized(ht->base_size);

	FOR_TABLE(i, item, ht) {
		inc_ref(&item->key);
		inc_ref(&item->value);
		table_insert(new_ht->data, item->key, item->value);
	}

	vm_push((struct VM *)S, YASL_TABLE(new_ht));
	return 0;
}

int table_clear(struct YASL_State *S) {
	ASSERT_TYPE((struct VM *)S, Y_TABLE, "table.clear");
	struct Table* ht = YASL_GETTABLE(vm_peek((struct VM *)S));
	inc_ref(&vm_peek((struct VM *)S));
	FOR_TABLE(i, item, ht) {
		del_item(item);
	}

	ht->count = 0;
	ht->size = HT_BASESIZE;
	free(ht->items);
	ht->items = calloc((size_t) ht->size, sizeof(Item_t));
	dec_ref(&vm_peek((struct VM *)S));
	vm_pop((struct VM *)S);
	vm_pushundef((struct VM *)S);
	return 0;
}
