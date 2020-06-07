#include "list_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "data-structures/YASL_List.h"
#include "yasl_error.h"
#include "yasl_state.h"

int list___get(struct YASL_State *S) {
	struct YASL_Object index = vm_pop((struct VM *) S);
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.__get", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *ls = YASL_GETLIST(vm_peek((struct VM *) S));
	if (!obj_isint(&index)) {
		return YASL_TYPE_ERROR;
	} else if (YASL_GETINT(index) < -(int64_t) ls->count || YASL_GETINT(index) >= (int64_t) ls->count) {
		return YASL_VALUE_ERROR;
	} else {
		if (index.value.ival >= 0) {
			vm_pop((struct VM *) S);
			vm_push((struct VM *) S, ls->items[YASL_GETINT(index)]);
		} else {
			vm_pop((struct VM *) S);
			vm_push((struct VM *) S, ls->items[YASL_GETINT(index) + ls->count]);
		}
	}
	return YASL_SUCCESS;
}

int list___set(struct YASL_State *S) {
	struct YASL_Object value = vm_pop((struct VM *) S);
	if (!YASL_isint(S)) {
		YASLX_print_err_bad_arg_type(S, "list.__set", 1, "int", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}

	yasl_int index = YASL_popint(S);
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.__set", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}

	struct YASL_List *ls = vm_poplist((struct VM *) S);

	if (index < -(yasl_int) ls->count || index >= (yasl_int) ls->count) {
		vm_print_err_value(&S->vm, "unable to index list of length %" PRI_SIZET " with index %" PRId64 ".", ls->count, index);
		return YASL_VALUE_ERROR;
	}
	if (index >= 0) ls->items[index] = value;
	else ls->items[index + ls->count] = value;

	return YASL_SUCCESS;
}

int table_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count);

int list_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count) {
	size_t string_count = 0;
	size_t string_size = 8;
	char *string = (char *) malloc(string_size);

	string[string_count++] = '[';
	struct YASL_List *list = vm_peeklist((struct VM *) S, S->vm.sp);
	if (list->count == 0) {
		vm_pop((struct VM *) S);
		string[string_count++] = ']';
		vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized_heap(0, string_count, string)));
		return YASL_SUCCESS;
	}

	FOR_LIST(i, obj, list) {
		vm_push((struct VM *) S, obj);

		if (vm_islist((struct VM *) S, S->vm.sp)) {
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
				size_t tmp_buffer_size = buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
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
				size_t tmp_buffer_size = buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = (void **) malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_count] = vm_peeklist((struct VM *) S, S->vm.sp);
				table_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_count + 1);
				free(tmp_buffer);
			}
		} else {
			vm_stringify_top((struct VM *) S);
		}

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
	vm_pop((struct VM *) S);

	string_count -= 2;
	string[string_count++] = ']';
	vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized_heap(0, string_count, string)));

	return YASL_SUCCESS;
}

int list_tostr(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.tostr", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	void **buffer = (void **) malloc(8 * sizeof(void *));
	buffer[0] = vm_peeklist((struct VM *) S, S->vm.sp);
	list_tostr_helper(S, buffer, 8, 1);
	free(buffer);
	return YASL_SUCCESS;
}

int list_push(struct YASL_State *S) {
	struct YASL_Object val = vm_pop((struct VM *) S);
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.push", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	YASL_List_append(YASL_GETLIST(vm_peek((struct VM *) S)), val);
	return YASL_SUCCESS;
}

int list_copy(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.copy", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *ls = YASL_GETLIST(vm_pop((struct VM *) S));
	struct RC_UserData *new_ls = rcls_new_sized(ls->size);
	struct YASL_List *new_list = (struct YASL_List *) new_ls->data;
	FOR_LIST(i, elmt, ls) {
		YASL_List_append(new_list, elmt);
	}

	vm_pushlist((struct VM *) S, new_ls);
	return YASL_SUCCESS;
}

static struct RC_UserData *list_concat(struct YASL_List *a, struct YASL_List *b) {
	size_t size = a->count + b->count;
	struct RC_UserData *ptr = rcls_new_sized(size);
	for (size_t i = 0; i < a->count; i++) {
		YASL_List_append((struct YASL_List *) ptr->data, (a)->items[i]);
	}
	for (size_t i = 0; i < (b)->count; i++) {
		YASL_List_append((struct YASL_List *) ptr->data, (b)->items[i]);
	}

	return ptr;
}

int list___add(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.__add", 1, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *b = YASL_GETLIST(vm_pop((struct VM *) S));
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.__add", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *a = YASL_GETLIST(vm_pop((struct VM *) S));

	vm_pushlist((struct VM *) S, list_concat(a, b));
	return YASL_SUCCESS;
}

int list_extend(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.extend", 1, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *extend_ls = YASL_GETLIST(vm_pop((struct VM *) S));
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.extend", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *ls = YASL_GETLIST(vm_pop((struct VM *) S));

	struct YASL_List *exls = extend_ls;

	FOR_LIST(i, obj, exls) {
		YASL_List_append(ls, obj);
	}

	vm_pushundef((struct VM *) S);
	return YASL_SUCCESS;
}

int list_pop(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.pop", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *ls = YASL_GETLIST(vm_pop((struct VM *) S));
	if (ls->count == 0) {
		vm_print_err((struct VM *)S, "ValueError: %s expected nonempty list as arg 0.", "list.pop");
		return YASL_VALUE_ERROR;
	}
	vm_push((struct VM *) S, ls->items[--ls->count]);
	return YASL_SUCCESS;
}

int list_search(struct YASL_State *S) {
	struct YASL_Object needle = vm_pop((struct VM *) S);
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.search", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *haystack = YASL_GETLIST(vm_pop((struct VM *) S));
	struct YASL_Object index = YASL_UNDEF();

	FOR_LIST(i, obj, haystack) {
		if (!isfalsey(isequal(obj, needle)))
			index = YASL_INT((yasl_int) i);
	}

	vm_push((struct VM *) S, index);
	return YASL_SUCCESS;
}

int list_reverse(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.reverse", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *ls = vm_poplist((struct VM *) S);
	YASL_reverse(ls);
	vm_pushundef((struct VM *) S);
	return YASL_SUCCESS;
}

int list_clear(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.clear", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *list = vm_poplist((struct VM *) S);
	FOR_LIST(i, obj, list) dec_ref(&obj);
	list->count = 0;
	list->size = LIST_BASESIZE;
	list->items = (struct YASL_Object *) realloc(list->items, sizeof(struct YASL_Object) * list->size);
	vm_pushundef((struct VM *) S);
	return YASL_SUCCESS;
}

int list_join(struct YASL_State *S) {
	if (!vm_isstr((struct VM *) S)) {
		YASLX_print_err_bad_arg_type(S, "list.join", 1, "str", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_String *string = vm_peekstr((struct VM *) S, S->vm.sp);
	S->vm.sp--;
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.join", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *list = vm_peeklist((struct VM *) S, S->vm.sp);
	S->vm.sp++;

	size_t buffer_count = 0;
	size_t buffer_size = 8;
	char *buffer = (char *) malloc(buffer_size);

	if (list->count == 0) {
		vm_pushstr((struct VM *) S, YASL_String_new_sized(0, ""));
		vm_pop((struct VM *) S);
		vm_pop((struct VM *) S);
		return YASL_SUCCESS;
	}

	vm_push((struct VM *) S, list->items[0]);
	enum YASL_Types index = vm_peek((struct VM *) S, S->vm.sp).type;
	struct YASL_Object key = YASL_STR(YASL_String_new_sized(strlen("tostr"), "tostr"));
	struct YASL_Object result = YASL_Table_search(S->vm.builtins_htable[index], key);
	str_del(YASL_GETSTR(key));
	YASL_GETCFN(result)->value(S);
	struct YASL_String *str = vm_popstr((struct VM *) S);

	while (buffer_count + YASL_String_len(str) >= buffer_size) {
		buffer_size *= 2;
		buffer = (char *) realloc(buffer, buffer_size);
	}

	memcpy(buffer + buffer_count, str->str + str->start, YASL_String_len(str));
	buffer_count += YASL_String_len(str);


	for (size_t i = 1; i < list->count; i++) {
		while (buffer_count + YASL_String_len(string) >= buffer_size) {
			buffer_size *= 2;
			buffer = (char *) realloc(buffer, buffer_size);
		}

		memcpy(buffer + buffer_count, string->str + string->start, YASL_String_len(string));
		buffer_count += YASL_String_len(string);

		vm_push((struct VM *) S, list->items[i]);
		enum YASL_Types index = vm_peek((struct VM *) S, S->vm.sp).type;
		struct YASL_Object key = YASL_STR(YASL_String_new_sized(strlen("tostr"), "tostr"));
		struct YASL_Object result = YASL_Table_search(S->vm.builtins_htable[index], key);
		str_del(YASL_GETSTR(key));
		YASL_GETCFN(result)->value(S);
		struct YASL_String *str = vm_popstr((struct VM *) S);

		while (buffer_count + YASL_String_len(str) >= buffer_size) {
			buffer_size *= 2;
			buffer = (char *) realloc(buffer, buffer_size);
		}

		memcpy(buffer + buffer_count, str->str + str->start, YASL_String_len(str));
		buffer_count += YASL_String_len(str);
	}
	vm_pop((struct VM *) S);
	vm_pop((struct VM *) S);
	vm_pushstr((struct VM *) S, YASL_String_new_sized_heap(0, buffer_count, buffer));
	return YASL_SUCCESS;
}

const int SORT_TYPE_EMPTY = 0;
const int SORT_TYPE_STR = -1;
const int SORT_TYPE_NUM = 1;
void sort(struct YASL_Object *list, const size_t len) {
	// Base cases
	struct YASL_Object tmpObj;
	if (len < 2) return;
	if (len == 2) {
		if (yasl_object_cmp(list[0], list[1]) > 0) {
			tmpObj = list[0];
			list[0] = list[1];
			list[1] = tmpObj;
		}
		return;
	}

	// YASL_Set sorting bounds
	size_t left = 0;
	size_t right = len - 1;

	// Determine random midpoint to use (good average case)
	const size_t randIndex = rand() % len;
	const struct YASL_Object mid = list[randIndex];

	// Determine exact number of items less than mid (mid's index)
	// Furthermore, ensure list is not homogenous to avoid infinite loops
	size_t ltCount = 0;
	int seenDifferent = 0;
	for (size_t i = 0; i < len; i++) {
		if (yasl_object_cmp(list[i], mid) < 0) ltCount++;
		if (seenDifferent == 0 && yasl_object_cmp(list[0], list[i]) != 0) seenDifferent = 1;
	}
	if (seenDifferent == 0) return;

	// Ensure all items are on the correct side of mid
	while (left < right) {
		while (yasl_object_cmp(list[left], mid) < 0) left++;
		while (yasl_object_cmp(list[right], mid) >= 0) {
			if (right == 0) break;
			right--;
		}

		int cmp = yasl_object_cmp(list[left], list[right]);
		if (cmp > 0 && left < right) {
			tmpObj = list[right];
			list[right] = list[left];
			list[left++] = tmpObj;
			if (right == 0) break;
			right--;
		} else if (cmp == 0) {
			left++;
			if (right == 0) break;
			right--;
		}
	}

	// Let sort() finish that for us...
	sort(list, ltCount);
	sort(&list[ltCount], len - ltCount);
}

int list_sort(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.sort", 0, "list", YASL_TYPE_NAMES[YASL_peektype(S)]);
		return YASL_TYPE_ERROR;
	}
	struct YASL_List *list = vm_poplist((struct VM *) S);
	int type = SORT_TYPE_EMPTY;

	int err = 0;
	for (size_t i = 0; i < list->count; i++) {
		switch (list->items[i].type) {
		case Y_STR:
			if (type == SORT_TYPE_EMPTY) {
				type = SORT_TYPE_STR;
			} else if (type == SORT_TYPE_NUM) {
				err = -1;
			}
			break;
		case Y_INT:
		case Y_FLOAT:
			if (type == SORT_TYPE_EMPTY) {
				type = SORT_TYPE_NUM;
			} else if (type == SORT_TYPE_STR) {
				err = -1;
			}
			break;
		default: err = -1;
		}

		if (err != 0) {
			vm_print_err((struct VM *)S, "ValueError: %s expected a list of all numbers or all strings.", "list.sort");
			return YASL_VALUE_ERROR;
		}
	}

	if (type != SORT_TYPE_EMPTY) {
		sort(list->items, list->count);
	}

	vm_pushundef((struct VM *) S);
	return YASL_SUCCESS;
}
