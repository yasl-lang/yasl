#include "list_methods.h"

#include <stdio.h>

#include "VM.h"
#include "YASL_Object.h"
#include "list.h"
#include "yasl_state.h"

int list___get(struct YASL_State *S) {
    struct YASL_Object index = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.__get");
    struct List *ls = YASL_GETLIST(vm_peek(S->vm));
    if (!YASL_ISINT(index)) {
        S->vm->sp++;
        return -1;
    } else if (YASL_GETINT(index) < -ls->count || YASL_GETINT(index) >= ls->count) {
        printf("IndexError\n");
        return -1;
    } else {
        if (index.value.ival >= 0) {
            vm_pop(S->vm);
            vm_push(S->vm, ls->items[YASL_GETINT(index)]);
        }
        else {
            vm_pop(S->vm);
            vm_push(S->vm, ls->items[YASL_GETINT(index) + ls->count]);
        }
    }
    return 0;
}

int list___set(struct YASL_State *S) {
    struct YASL_Object value = vm_pop(S->vm);
    struct YASL_Object index = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.__set");
    struct List *ls = YASL_GETLIST(vm_pop(S->vm));
    if (!YASL_ISINT(index)) {
        printf("TypeError: cannot index list with non-integer\n");
        vm_push(S->vm, YASL_UNDEF());
        return -1;
    } else if (YASL_GETINT(index) < -ls->count || YASL_GETINT(index) >= ls->count) {
        printf("%d || %d\n", YASL_GETINT(index) < -ls->count, YASL_GETINT(index) >= ls->count);
        printf("IndexError\n");
        vm_push(S->vm, YASL_UNDEF());
        return -1;
    } else {
        if (YASL_GETINT(index) >= 0) ls->items[YASL_GETINT(index)] = value;
        else ls->items[YASL_GETINT(index) + ls->count] = value;
        vm_push(S->vm, value);
    }
    return 0;
}

int table_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count);

int list_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count) {
	size_t string_count = 0;
	size_t string_size = 8;
	char *string = malloc(string_size);

	string[string_count++] = '[';
	struct List *list = vm_peeklist(S->vm, S->vm->sp);
	if (list->count == 0)    {
		vm_pop(S->vm);
		string[string_count++] = ']';

		vm_push(S->vm, YASL_STR(str_new_sized_heap(0, string_count, string)));
		return 0;
	}

	FOR_LIST(i, obj, list) {
		vm_push(S->vm, obj);

		if (YASL_ISLIST(VM_PEEK(S->vm, S->vm->sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist(S->vm, S->vm->sp)) {
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
				tmp_buffer[buffer_size] = vm_peeklist(S->vm, S->vm->sp);
				list_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
			}
		} else if (YASL_ISTABLE(VM_PEEK(S->vm, S->vm->sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist(S->vm, S->vm->sp)) {
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
				tmp_buffer[buffer_count] = vm_peeklist(S->vm, S->vm->sp);
				table_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
				free(tmp_buffer);
			}
		} else {
			vm_stringify_top(S->vm);
		}

		String_t *str = vm_popstr(S->vm);
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

	string_count -= 2;
	string[string_count++] = ']';

	vm_push(S->vm, YASL_STR(str_new_sized_heap(0, string_count, string)));

	return 0;
}

int list_tostr(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_LIST, "list.tostr");

	void **buffer = malloc(8*sizeof(void *));
	buffer[0] = vm_peeklist(S->vm, S->vm->sp);
	list_tostr_helper(S, buffer, 8, 1);
	free(buffer);

	return 0;
}

int list_push(struct YASL_State *S) {
	struct YASL_Object val = vm_pop(S->vm);
	ASSERT_TYPE(S->vm, Y_LIST, "list.push");
	ls_append(YASL_GETLIST(vm_peek(S->vm)), val);
	return 0;
}

int list_copy(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_LIST, "list.copy");
	struct List *ls = YASL_GETLIST(vm_pop(S->vm));
	struct RC_UserData *new_ls = ls_new_sized(ls->size);
	((struct List *) new_ls->data)->count = ls->count;
	memcpy(((struct List *) new_ls->data)->items, ls->items,
	       ((struct List *) new_ls->data)->count * sizeof(struct YASL_Object));

	vm_pushlist(S->vm, new_ls);
	return 0;
}

int list_extend(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.extend");
    struct List *extend_ls = YASL_GETLIST(vm_pop(S->vm));
    ASSERT_TYPE(S->vm, Y_LIST, "list.extend");
    struct List *ls  = YASL_GETLIST(vm_pop(S->vm));

    struct List *exls = extend_ls;

    FOR_LIST(i, obj, exls) {
    	ls_append(ls, obj);
    }

    vm_pushundef(S->vm);
    return 0;
}

int list_pop(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_LIST, "list.pop");
	struct List *ls = YASL_GETLIST(vm_pop(S->vm));
	if (ls->count == 0) {
		puts("cannot pop from empty list.");
		exit(EXIT_FAILURE);
	}
	vm_push(S->vm, ls->items[--ls->count]);
	return 0;
}

int list_search(struct YASL_State *S) {
	struct YASL_Object needle = vm_pop(S->vm);
	ASSERT_TYPE(S->vm, Y_LIST, "list.search");
	struct List *haystack = YASL_GETLIST(vm_pop(S->vm));
	struct YASL_Object index = YASL_UNDEF();

	FOR_LIST(i, obj, haystack) {
		if (!isfalsey(isequal(obj, needle)))
			index = YASL_INT(i);
	}

	vm_push(S->vm, index);
	return 0;
}

int list_reverse(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_LIST, "list.reverse");
	struct List *ls = vm_poplist(S->vm);
	ls_reverse(ls);
	vm_pushundef(S->vm);
	return 0;
}

int list_slice(struct YASL_State *S) {
	struct YASL_Object end_index = vm_pop(S->vm);
	struct YASL_Object start_index = vm_pop(S->vm);
	ASSERT_TYPE(S->vm, Y_LIST, "list.slice");
	struct List *list = vm_poplist(S->vm);
	if (!YASL_ISINT(start_index) || !YASL_ISINT(end_index)) {
		return -1;
	} else if (YASL_GETINT(start_index) < -list->count ||
		   YASL_GETINT(start_index) > list->count) {
		return -1;
	} else if (YASL_GETINT(end_index) < -list->count || YASL_GETINT(end_index) > list->count) {
		return -1;
	}

	int64_t start = YASL_GETINT(start_index) < 0 ? YASL_GETINT(start_index) + list->count : YASL_GETINT(
		start_index);
	int64_t end =
		YASL_GETINT(end_index) < 0 ? YASL_GETINT(end_index) + list->count : YASL_GETINT(end_index);

	if (start > end) {
		return -1;
	}

	struct RC_UserData *new_list = ls_new_sized(end - start);

	for (int64_t i = start; i < end; i++) {
		ls_append(new_list->data, list->items[i]); // = list->items[i];
		inc_ref(list->items + i);
	}

	vm_pushlist(S->vm, new_list);

	return 0;
}

int list_clear(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_LIST, "list.clear");
	struct List *list = vm_poplist(S->vm);
	FOR_LIST(i, obj, list) dec_ref(&obj);
	list->count = 0;
	list->size = LS_BASESIZE;
	list->items = realloc(list->items, sizeof(struct YASL_Object)*list->size);
	vm_pushundef(S->vm);
	return 0;
}

int list_join(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_STR, "list.join");
	String_t *string = vm_peekstr(S->vm, S->vm->sp);
	S->vm->sp--;
	ASSERT_TYPE(S->vm, Y_LIST, "list.join");
	struct List *list = vm_peeklist(S->vm, S->vm->sp);
	S->vm->sp++;

	size_t buffer_count = 0;
	size_t buffer_size = 8;
	char *buffer = malloc(buffer_size);

	if (list->count == 0) {
		vm_pushstr(S->vm, str_new_sized(0, ""));
		vm_pop(S->vm);
		vm_pop(S->vm);
		return 0;
	}

	vm_push(S->vm, list->items[0]);
	YASL_Types index = VM_PEEK(S->vm, S->vm->sp).type;
	struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
	struct YASL_Object result = table_search(S->vm->builtins_htable[index], key);
	str_del(YASL_GETSTR(key));
	YASL_GETCFN(result)->value(S);
	String_t *str = vm_popstr(S->vm);

	while (buffer_count + yasl_string_len(str) >= buffer_size) {
		buffer_size *= 2;
		buffer = realloc(buffer, buffer_size);
	}

	memcpy(buffer + buffer_count, str->str + str->start, yasl_string_len(str));
	buffer_count += yasl_string_len(str);


	for (int64_t i = 1; i < list->count; i++) {
		while (buffer_count + yasl_string_len(string) >= buffer_size) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size);
		}

		memcpy(buffer + buffer_count, string->str + string->start, yasl_string_len(string));
		buffer_count += yasl_string_len(string);

		vm_push(S->vm, list->items[i]);
		YASL_Types index = VM_PEEK(S->vm, S->vm->sp).type;
		struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
		struct YASL_Object result = table_search(S->vm->builtins_htable[index], key);
		str_del(YASL_GETSTR(key));
		YASL_GETCFN(result)->value(S);
		String_t *str = vm_popstr(S->vm);

		while (buffer_count + yasl_string_len(str) >= buffer_size) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size);
		}

		memcpy(buffer + buffer_count, str->str + str->start, yasl_string_len(str));
		buffer_count += yasl_string_len(str);
	}
	vm_pop(S->vm);
	vm_pop(S->vm);
	vm_pushstr(S->vm, str_new_sized_heap(0, buffer_count, buffer));
	return 0;
}