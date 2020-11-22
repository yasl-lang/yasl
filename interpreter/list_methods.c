#include "list_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "data-structures/YASL_List.h"
#include "yasl_error.h"
#include "yasl_state.h"

static struct YASL_List *YASLX_checklist(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_islist(S)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type list, got arg of type %s.",
				  name, pos, YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return (struct YASL_List *)YASL_popuserdata(S);
}

void list___len(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checklist(S, "list.__len", 0);
	YASL_pushint(S, YASL_List_length(ls));
}

void list___get(struct YASL_State *S) {
	yasl_int index = YASLX_checkint(S, "list.__get", 1);
	struct YASL_List *ls = YASLX_checklist(S, "list.__get", 0);

	if (index < -(int64_t) ls->count || index >= (int64_t) ls->count) {
		vm_print_err_value(&S->vm, "unable to index list of length %" PRI_SIZET " with index %" PRI_SIZET ".", ls->count, index);
		YASL_throw_err(S, YASL_VALUE_ERROR);
	} else {
		if (index >= 0) {
			vm_push((struct VM *) S, ls->items[index]);
		} else {
			vm_push((struct VM *) S, ls->items[index + ls->count]);
		}
	}
}

void list___set(struct YASL_State *S) {
	struct YASL_Object value = vm_pop((struct VM *) S);
	yasl_int index = YASLX_checkint(S, "list.__set", 1);
	struct YASL_List *ls = YASLX_checklist(S, "list.__set", 0);

	if (index < -(yasl_int) ls->count || index >= (yasl_int) ls->count) {
		vm_print_err_value(&S->vm, "unable to index list of length %" PRI_SIZET " with index %" PRId64 ".", ls->count, index);
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}
	if (index >= 0) ls->items[index] = value;
	else ls->items[index + ls->count] = value;
}

int table_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count);
bool buffer_contains(void **buffer, size_t buffer_count, void *val);
void rec_call(struct YASL_State *S, void **buffer, const size_t buffer_count, const size_t buffer_size, int (*f)(struct YASL_State *, void **, size_t, size_t));

#define FOUND_LIST "[...], "
#define FOUND_TABLE "{...}, "

int list_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count) {
	struct YASL_ByteBuffer bb = NEW_BB(8);

	YASL_ByteBuffer_add_byte(&bb, '[');
	struct YASL_List *list = vm_peeklist((struct VM *) S);
	if (list->count == 0) {
		vm_pop((struct VM *) S);
		YASL_ByteBuffer_add_byte(&bb, ']');
		vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized_heap(0, bb.count, (char *)bb.bytes)));
		return YASL_SUCCESS;
	}

	FOR_LIST(i, obj, list) {
		vm_push((struct VM *) S, obj);

		if (vm_islist((struct VM *) S)) {
			bool found = buffer_contains(buffer, buffer_count, vm_peeklist((struct VM *) S));
			if (found) {
				YASL_ByteBuffer_extend(&bb, (unsigned char *)FOUND_LIST, strlen(FOUND_LIST));
				vm_pop((struct VM *) S);
				continue;
			} else {
				rec_call(S, buffer, buffer_count, buffer_size, &list_tostr_helper);
			}
		} else if (vm_istable((struct VM *) S)) {
			bool found = buffer_contains(buffer, buffer_count, vm_peeklist((struct VM *) S));
			if (found) {
				YASL_ByteBuffer_extend(&bb, (unsigned char *)FOUND_TABLE, strlen(FOUND_TABLE));
				vm_pop((struct VM *) S);
				continue;
			} else {
				rec_call(S, buffer, buffer_count, buffer_size, &table_tostr_helper);
			}
		} else {
			vm_stringify_top((struct VM *) S);
		}

		struct YASL_String *str = vm_popstr((struct VM *) S);
		YASL_ByteBuffer_extend(&bb, (unsigned char *)str->str + str->start, YASL_String_len(str));
		YASL_ByteBuffer_extend(&bb, (unsigned char *)", ", strlen(", "));
	}
	vm_pop((struct VM *) S);

	bb.count -= 2;
	YASL_ByteBuffer_add_byte(&bb, ']');

	vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized_heap(0, bb.count, (char *)bb.bytes)));

	return YASL_SUCCESS;
}

void list_tostr(struct YASL_State *S) {
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.tostr", 0, "list", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	void **buffer = (void **) malloc(8 * sizeof(void *));
	buffer[0] = vm_peeklist((struct VM *) S, S->vm.sp);
	list_tostr_helper(S, buffer, 8, 1);
	free(buffer);
}

void list_push(struct YASL_State *S) {
	struct YASL_Object val = vm_pop((struct VM *) S);
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.push", 0, "list", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	YASL_List_append(YASL_GETLIST(vm_peek((struct VM *) S)), val);
}

void list_copy(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checklist(S, "list.copy", 0);
	struct RC_UserData *new_ls = rcls_new_sized(ls->size);
	struct YASL_List *new_list = (struct YASL_List *) new_ls->data;
	FOR_LIST(i, elmt, ls) {
		YASL_List_append(new_list, elmt);
	}

	vm_pushlist((struct VM *) S, new_ls);
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

void list___add(struct YASL_State *S) {
	struct YASL_List *b = YASLX_checklist(S, "list.__add", 1);
	struct YASL_List *a = YASLX_checklist(S, "list.__add", 0);

	vm_pushlist((struct VM *) S, list_concat(a, b));
}

void list___eq(struct YASL_State *S) {
	struct YASL_List *right = YASLX_checklist(S, "list.__eq", 1);
	struct YASL_List *left = YASLX_checklist(S, "list.__eq", 0);

	if (left->count != right->count) {
		YASL_pushbool(S, false);
		return;
	}

	for (size_t i = 0; i < left->count; i++) {
		vm_push((struct VM *)S, left->items[i]);
		vm_push((struct VM *)S, right->items[i]);
		vm_EQ((struct VM *)S);
		if (!YASL_popbool(S)) {
			YASL_pushbool(S, false);
			return;
		}
	}
	YASL_pushbool(S, true);
}

void list_pop(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checklist(S, "list.pop", 0);
	if (ls->count == 0) {
		vm_print_err((struct VM *)S, "ValueError: %s expected nonempty list as arg 0.", "list.pop");
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}
	vm_push((struct VM *) S, ls->items[--ls->count]);
}

void list_search(struct YASL_State *S) {
	struct YASL_Object needle = vm_pop((struct VM *) S);
	struct YASL_List *haystack = YASLX_checklist(S, "list.search", 0);
	struct YASL_Object index = YASL_UNDEF();

	FOR_LIST(i, obj, haystack) {
		if ((isequal(&obj, &needle)))
			index = YASL_INT((yasl_int) i);
	}

	vm_push((struct VM *) S, index);
}

void list_reverse(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checklist(S, "list.reverse", 0);
	YASL_reverse(ls);
	YASL_pushundef(S);
}

void list_clear(struct YASL_State *S) {
	struct YASL_List *list = YASLX_checklist(S, "list.clear", 0);
	FOR_LIST(i, obj, list) dec_ref(&obj);
	list->count = 0;
	list->size = LIST_BASESIZE;
	list->items = (struct YASL_Object *) realloc(list->items, sizeof(struct YASL_Object) * list->size);
	YASL_pushundef(S);
}

void list_join(struct YASL_State *S) {
	if (!vm_isstr((struct VM *) S)) {
		YASLX_print_err_bad_arg_type(S, "list.join", 1, "str", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	struct YASL_String *string = vm_peekstr((struct VM *) S, S->vm.sp);
	S->vm.sp--;
	if (!YASL_islist(S)) {
		YASLX_print_err_bad_arg_type(S, "list.join", 0, "list", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
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
		return;
	}

	vm_push((struct VM *)S, list->items[0]);
	vm_stringify_top((struct VM *)S);
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
		vm_stringify_top((struct VM *)S);
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

// TODO: clean this up
void list_sort(struct YASL_State *S) {
	struct YASL_List *list = YASLX_checklist(S, "list.sort", 0);
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
			YASL_throw_err(S, YASL_VALUE_ERROR);
		}
	}

	if (type != SORT_TYPE_EMPTY) {
		sort(list->items, list->count);
	}

	YASL_pushundef(S);
}
