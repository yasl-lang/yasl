#include "table_methods.h"

#include <stdio.h>

#include "yasl_state.h"
#include "VM.h"
#include "YASL_Object.h"

int table___get(struct YASL_State *S) {
    struct YASL_Object key = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_TABLE, "table.__get");
    struct Table* ht = YASL_GETTBL(PEEK(S->vm));
    struct YASL_Object *result = table_search(ht, key);
    if (result == NULL) {
        S->vm->sp++;  // TODO: fix this
        //vm_push(S->vm, key);
        return -1;
    }
    else {
        vm_pop(S->vm);
        vm_push(S->vm, *result);
    }
    return 0;
}

int table___set(struct YASL_State *S) {
	struct YASL_Object val = vm_pop(S->vm);
	struct YASL_Object key = vm_pop(S->vm);
	ASSERT_TYPE(S->vm, Y_TABLE, "table.__set");
	struct Table *ht = YASL_GETTBL(vm_pop(S->vm));

	if (YASL_ISLIST(key) || YASL_ISTBL(key) || YASL_ISUSERDATA(key)) {
		printf("Error: unable to use mutable object of type %x as key.\n", key.type);
		return -1;
	}
	table_insert(ht, key, val);
	vm_push(S->vm, val);
	return 0;
}

int object_tostr(struct YASL_State *S) {
	YASL_Types index = VM_PEEK(S->vm, S->vm->sp).type;
	struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
	struct YASL_Object *result = table_search(S->vm->builtins_htable[index], key);
	str_del(YASL_GETSTR(key));
	YASL_GETCFN(*result)->value(S);
	return 0;
}

int list_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count);

int table_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count) {
	size_t string_count = 0;
	size_t string_size = 8;
	char *string = malloc(string_size);

	string[string_count++] = '{';
	struct Table *table = vm_peektable(S->vm, S->vm->sp);
	if (table->count == 0) {
		string[string_count++] = '}';
		vm_push(S->vm, YASL_STR(str_new_sized_heap(0, string_count, string)));
		return 0;
	}

	for (size_t i = 0; i < table->size; i++) {
		Item_t *item = table->items[i];
		if (item == NULL || item == &TOMBSTONE) continue;

		vm_push(S->vm, *item->key);

		object_tostr(S);

		String_t *str = vm_popstr(S->vm);
		while (string_count + yasl_string_len(str) >= string_size) {
			string_size *= 2;
			string = realloc(string, string_size);
		}

		memcpy(string + string_count, str->str + str->start, yasl_string_len(str));
		string_count += yasl_string_len(str);
		// str_del(str);

		if (string_count + 2 >= string_size) {
			string_size *= 2;
			string = realloc(string, string_size);
		}

		string[string_count++] = ':';
		string[string_count++] = ' ';

		vm_push(S->vm, *item->value);

		if (YASL_ISLIST(VM_PEEK(S->vm, S->vm->sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist(S->vm, S->vm->sp)) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (string_count + 7 >= string_size) {
					string_size *= 2;
					string = realloc(string, string_size);
				}
				string[string_count++] = '[';
				string[string_count++] = '.';
				string[string_count++] = '.';
				string[string_count++] = '.';
				string[string_count++] = ']';
				string[string_count++] = ',';
				string[string_count++] = ' ';
				continue;
			} else {
				size_t tmp_buffer_size = buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_count] = vm_peeklist(S->vm, S->vm->sp);
				list_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
				free(tmp_buffer);
			}
		} else if (YASL_ISTBL(VM_PEEK(S->vm, S->vm->sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist(S->vm, S->vm->sp)) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (string_count + 7 >= string_size) {
					string_size *= 2;
					string = realloc(string, string_size);
				}
				string[string_count++] = '{';
				string[string_count++] = '.';
				string[string_count++] = '.';
				string[string_count++] = '.';
				string[string_count++] = '}';
				string[string_count++] = ',';
				string[string_count++] = ' ';
				continue;
			} else {
				size_t tmp_buffer_size = buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_size] = vm_peeklist(S->vm, S->vm->sp);
				table_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
			}
		} else {
			YASL_Types index = VM_PEEK(S->vm, S->vm->sp).type;
			struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
			struct YASL_Object *result = table_search(S->vm->builtins_htable[index], key);
			str_del(YASL_GETSTR(key));
			YASL_GETCFN(*result)->value(S);
		}

		str = vm_popstr(S->vm);
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

	vm_pop(S->vm);

	if (string_count + 2 >= string_size) {
		string_size *= 2;
		string = realloc(string, string_size);
	}

	string_count -= 2;
	string[string_count++] = '}';

	vm_push(S->vm, YASL_STR(str_new_sized_heap(0, string_count, string)));

	return 0;
}

int table_tostr(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_TABLE, "table.tostr");

	void **buffer = malloc(8*sizeof(void *));
	buffer[0] = vm_peektable(S->vm, S->vm->sp);
	table_tostr_helper(S, buffer, 8, 1);
	free(buffer);

	return 0;
}


int table_keys(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_TABLE, "table.keys");
    struct Table *ht = YASL_GETTBL(vm_pop(S->vm));
    struct RC_UserData* ls = ls_new();
    Item_t* item;
    for (size_t i = 0; i < ht->size; i++) {
        item = ht->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls->data, *(item->key));
        }
    }
    vm_push(S->vm, YASL_LIST(ls));
    return 0;
}

int table_values(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_TABLE, "table.values");
    struct Table *ht = YASL_GETTBL(vm_pop(S->vm));
    struct RC_UserData* ls = ls_new();
    Item_t* item;
    for (size_t i = 0; i < ht->size; i++) {
        item = ht->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            ls_append(ls->data, *(item->value));
        }
    }
    vm_push(S->vm, YASL_LIST(ls));
    return 0;
}

int table_clone(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_TABLE, "table.clone");
    struct Table* ht = YASL_GETTBL(vm_pop(S->vm));
    struct RC_UserData* new_ht = rcht_new_sized(ht->base_size);
    for (size_t i = 0; i < ht->size; i++) {
        Item_t* item = ht->items[i];
        if (item != NULL && item != &TOMBSTONE) {
            inc_ref(item->key);
            inc_ref(item->value);
            table_insert(new_ht->data, *item->key, *item->value);
        }
    }

    vm_push(S->vm, YASL_TBL(new_ht));
    return 0;
}

int table_clear(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_TABLE, "table.clear");
	struct Table* ht = YASL_GETTBL(vm_pop(S->vm));
	for (size_t i = 0; i < ht->size; i++) {
		Item_t* item = ht->items[i];
		if (item != NULL && item != &TOMBSTONE) {
			del_item(item);
		}
	}

	ht->count = 0;
	ht->size = HT_BASESIZE;
	free(ht->items);
	ht->items = calloc(ht->size, sizeof(Item_t*));
	vm_pushundef(S->vm);
	return 0;
}