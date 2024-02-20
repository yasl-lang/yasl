#include "list_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "data-structures/YASL_List.h"
#include "yasl_error.h"
#include "yasl_state.h"

static struct YASL_List *YASLX_checknlist(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnlist(S, pos)) {
		vm_print_err_type(&S->vm, "%s expected arg in position %d to be of type list, got arg of type %s.",
				  name, pos, YASL_peekntypename(S, pos));
		YASLX_throw_type_err(S);
	}
	return (struct YASL_List *)YASL_peeknuserdata(S, pos);
}

void list___get_helper(struct YASL_State *S, struct YASL_List *ls, yasl_int index) {
	if (index < -(int64_t) ls->count || index >= (int64_t) ls->count) {
		vm_print_err_value(&S->vm, "unable to index list of length %" PRI_SIZET " with index %" PRI_SIZET ".", ls->count, index);
		YASLX_throw_value_err(S);
	} else {
		if (index >= 0) {
			vm_push((struct VM *) S, ls->items[index]);
		} else {
			vm_push((struct VM *) S, ls->items[index + ls->count]);
		}
	}
}

int list___get(struct YASL_State *S) {
	yasl_int index = YASLX_checknint(S, "list.__get", 1);
	struct YASL_List *ls = YASLX_checknlist(S, "list.__get", 0);

	list___get_helper(S, ls, index);
	return 1;
}

int list___len(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.__len", 0);
	YASL_pushint(S, YASL_List_length(ls));
	return 1;
}

int list___set(struct YASL_State *S) {
	struct YASL_Object value = vm_pop((struct VM *) S);
	yasl_int index = YASLX_checknint(S, "list.__set", 1);
	struct YASL_List *ls = YASLX_checknlist(S, "list.__set", 0);

	if (index < -(yasl_int) ls->count || index >= (yasl_int) ls->count) {
		vm_print_err_value(&S->vm, "unable to index list of length %" PRI_SIZET " with index %" PRId64 ".", ls->count, index);
		YASLX_throw_value_err(S);
	}

	if (index < 0) index += ls->count;

	inc_ref(&value);
	vm_dec_ref(&S->vm, ls->items + index);
	ls->items[index] = value;
	return 1;
}

static int list___next(struct YASL_State *S) {
	yasl_int curr = YASLX_checknint(S, "list.__next", 1);
	struct YASL_List *ls = YASLX_checknlist(S, "list.__next", 0);

	if (curr < -(yasl_int) ls->count || curr >= (yasl_int)ls->count) {
		YASL_pushbool(S, false);
		return 1;
	}

	YASL_pushint(S, curr + 1);
	vm_push(&S->vm, ls->items[curr]);
	YASL_pushbool(S, true);
	return 3;
}

int list___iter(struct YASL_State *S) {
	YASLX_checknlist(S, "list.__iter", 0);
	YASL_pushcfunction(S, &list___next, 2);
	YASL_pushint(S, 0);
	return 2;
}

int list_copy(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.copy", 0);
	struct RC_UserData *new_ls = rcls_new_sized(ls->size);
	ud_setmt(&S->vm, new_ls, S->vm.builtins_htable[Y_LIST]);
	struct YASL_List *new_list = (struct YASL_List *) new_ls->data;
	FOR_LIST(i, elmt, ls) {
		YASL_List_push(new_list, elmt);
	}

	vm_pushlist((struct VM *) S, new_ls);
	return 1;
}

int list_push(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.push", 0);
	struct YASL_Object val = vm_pop((struct VM *) S);

	YASL_List_push(ls, val);
	return 1;
}

/*
int list_pushv(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.pushv", 0);
	yasl_int num_va_args = YASL_peekvargscount(S);

	for (yasl_int i = 0; i < num_va_args; i++) {
		struct YASL_Object val = vm_peek((struct VM *)S, ((struct VM *)S)->fp + 2 + i);
		YASL_List_push(ls, val);
	}

	for (yasl_int i = 0; i < num_va_args; i++) {
		YASL_pop(S);
	}

	YASL_pop(S);

	return 1;
}
*/

static struct RC_UserData *list_concat(struct YASL_State *S, struct YASL_List *a, struct YASL_List *b) {
	size_t size = a->count + b->count;
	struct RC_UserData *ptr = rcls_new_sized(size);
	ud_setmt(&S->vm, ptr, (&S->vm)->builtins_htable[Y_LIST]);
	for (size_t i = 0; i < a->count; i++) {
		YASL_List_push((struct YASL_List *) ptr->data, (a)->items[i]);
	}
	for (size_t i = 0; i < (b)->count; i++) {
		YASL_List_push((struct YASL_List *) ptr->data, (b)->items[i]);
	}

	return ptr;
}

int list___add(struct YASL_State *S) {
	struct YASL_List *b = YASLX_checknlist(S, "list.__add", 1);
	struct YASL_List *a = YASLX_checknlist(S, "list.__add", 0);

	vm_pushlist((struct VM *) S, list_concat(S, a, b));
	return 1;
}

int list___eq(struct YASL_State *S) {
	struct YASL_List *right = YASLX_checknlist(S, "list.__eq", 1);
	struct YASL_List *left = YASLX_checknlist(S, "list.__eq", 0);

	if (left->count != right->count) {
		YASL_pushbool(S, false);
		return 1;
	}

	for (size_t i = 0; i < left->count; i++) {
		vm_push((struct VM *)S, left->items[i]);
		vm_push((struct VM *)S, right->items[i]);
		vm_EQ((struct VM *)S);
		if (!YASL_popbool(S)) {
			YASL_pushbool(S, false);
			return 1;
		}
	}

	YASL_pushbool(S, true);
	return 1;
}

int list_pop(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.pop", 0);
	if (ls->count == 0) {
		vm_print_err_value((struct VM *)S, "%s expected nonempty list as arg 0.", "list.pop");
		YASLX_throw_value_err(S);
	}
	vm_push((struct VM *) S, ls->items[--ls->count]);
	return 1;
}

int list_reverse(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.reverse", 0);
	YASL_reverse(ls);

	return 0;
}

int list_remove(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.remove", 0);

	FOR_LIST(i, name, ls) {
		YASL_duptop(S);
		vm_push(&S->vm, name);
		vm_EQ(&S->vm);
		if (YASL_popbool(S)) {
			vm_dec_ref(&S->vm, &name);
			size_t remaining = ls->count - i;
			memmove(ls->items + i, ls->items + i + 1, remaining * sizeof(struct YASL_Object));
			ls->count--;
			break;
		}
	}

	YASL_pop(S);
	return 1;
}

int list_search(struct YASL_State *S) {
	struct YASL_Object needle = vm_pop((struct VM *) S);
	struct YASL_List *haystack = YASLX_checknlist(S, "list.search", 0);
	struct YASL_Object index = YASL_UNDEF();

	FOR_LIST(i, obj, haystack) {
		if ((isequal(&obj, &needle))) {
			index = YASL_INT((yasl_int) i);
			break;
		}
	}

	vm_push((struct VM *) S, index);
	return 1;
}

int table_tostr_helper(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format);
bool buffer_contains(BUFFER(ptr) buffer, void *val);
void rec_call(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format, int (*f)(struct YASL_State *, BUFFER(ptr), struct YASL_Object *));

#define FOUND_LIST "[...], "
#define FOUND_TABLE "{...}, "

int list_tostr_helper(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format) {
	YASL_ByteBuffer bb = NEW_BB(8);

	YASL_ByteBuffer_add_byte(&bb, '[');
	struct YASL_List *list = vm_peeklist((struct VM *) S);
	if (list->count == 0) {
		YASL_pop(S);
		YASL_ByteBuffer_add_byte(&bb, ']');
		vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized_heap(0, bb.count, (char *)bb.items)));
		return YASL_SUCCESS;
	}

	FOR_LIST(i, obj, list) {
		vm_push((struct VM *) S, obj);

		if (vm_islist((struct VM *) S)) {
			bool found = buffer_contains(buffer, vm_peeklist((struct VM *) S));
			if (found) {
				YASL_ByteBuffer_extend(&bb, (unsigned char *)FOUND_LIST, strlen(FOUND_LIST));
				YASL_pop(S);
				continue;
			} else {
				rec_call(S, buffer, format, &list_tostr_helper);
			}
		} else if (vm_istable((struct VM *) S)) {
			bool found = buffer_contains(buffer, vm_peeklist((struct VM *) S));
			if (found) {
				YASL_ByteBuffer_extend(&bb, (unsigned char *)FOUND_TABLE, strlen(FOUND_TABLE));
				YASL_pop(S);
				continue;
			} else {
				rec_call(S, buffer, format, &table_tostr_helper);
			}
		} else {
			vm_stringify_top_format((struct VM *) S, format);
		}

		struct YASL_String *str = vm_popstr((struct VM *) S);
		YASL_ByteBuffer_extend(&bb, (unsigned char *)YASL_String_chars(str), YASL_String_len(str));
		YASL_ByteBuffer_extend(&bb, (unsigned char *)", ", strlen(", "));
	}
	YASL_pop(S);

	bb.count -= 2;
	YASL_ByteBuffer_add_byte(&bb, ']');

	vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized_heap(0, bb.count, (char *)bb.items)));

	return YASL_SUCCESS;
}

int list_tostr(struct YASL_State *S) {
	struct YASL_List *list = YASLX_checknlist(S, "list.tostr", 0);
	struct YASL_Object format = vm_pop((struct VM *)S);
	inc_ref(&format);

	BUFFER(ptr) buffer;
	BUFFER_INIT(ptr)(&buffer, 8);
	BUFFER_PUSH(ptr)(&buffer, list);
	list_tostr_helper(S, buffer, &format);
	BUFFER_CLEANUP(ptr)(&buffer);

	dec_ref(&format);
	return 1;
}

int list_clear(struct YASL_State *S) {
	struct YASL_List *list = YASLX_checknlist(S, "list.clear", 0);
	FOR_LIST(i, obj, list) vm_dec_ref(&S->vm, &obj);
	list->count = 0;
	list->size = LIST_BASESIZE;
	list->items = (struct YASL_Object *) realloc(list->items, sizeof(struct YASL_Object) * list->size);

	return 0;
}

int list_join(struct YASL_State *S) {
	if (YASL_isundef(S)) {
		YASL_pop(S);
		vm_pushstr((struct VM *)S, YASL_String_new_sized(0, ""));
	}
	if (!vm_isstr((struct VM *) S)) {
		YASLX_print_err_bad_arg_type(S, "list.join", 1, "str", YASL_peekntypename(S, 1));
		YASLX_throw_type_err(S);
	}
	struct YASL_String *string = vm_peekstr((struct VM *) S, S->vm.sp);
	S->vm.sp--;
	if (!YASL_isnlist(S, 0)) {
		YASLX_print_err_bad_arg_type(S, "list.join", 0, "list", YASL_peekntypename(S, 0));
		YASLX_throw_type_err(S);
	}
	struct YASL_List *list = vm_peeklist((struct VM *) S, S->vm.sp);
	S->vm.sp++;

	if (list->count == 0) {
		vm_pushstr((struct VM *) S, YASL_String_new_sized(0, ""));
		return 1;
	}

	size_t buffer_count = 0;
	size_t buffer_size = 8;
	char *buffer = (char *) malloc(buffer_size);

	vm_push((struct VM *)S, list->items[0]);
	vm_stringify_top((struct VM *)S);
	struct YASL_String *str = vm_popstr((struct VM *) S);

	while (buffer_count + YASL_String_len(str) >= buffer_size) {
		buffer_size *= 2;
		buffer = (char *) realloc(buffer, buffer_size);
	}

	memcpy(buffer + buffer_count, YASL_String_chars(str), YASL_String_len(str));
	buffer_count += YASL_String_len(str);


	for (size_t i = 1; i < list->count; i++) {
		while (buffer_count + YASL_String_len(string) >= buffer_size) {
			buffer_size *= 2;
			buffer = (char *) realloc(buffer, buffer_size);
		}

		memcpy(buffer + buffer_count, YASL_String_chars(string), YASL_String_len(string));
		buffer_count += YASL_String_len(string);

		vm_push((struct VM *) S, list->items[i]);
		vm_stringify_top((struct VM *)S);
		struct YASL_String *str = vm_popstr((struct VM *) S);

		while (buffer_count + YASL_String_len(str) >= buffer_size) {
			buffer_size *= 2;
			buffer = (char *) realloc(buffer, buffer_size);
		}

		memcpy(buffer + buffer_count, YASL_String_chars(str), YASL_String_len(str));
		buffer_count += YASL_String_len(str);
	}
	YASL_pop(S);
	YASL_pop(S);
	vm_pushstr((struct VM *) S, YASL_String_new_sized_heap(0, buffer_count, buffer));
	return 1;
}

int list_spread(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.spread", 0);
	const yasl_int len = YASL_List_length(ls);

	FOR_LIST(i, elmt, ls) {
		vm_push((struct VM *)S, elmt);
	}

	return (int)len;
}

int list_count(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.count", 0);
	struct YASL_Object v = vm_pop(&S->vm);

	yasl_int count = 0;
	FOR_LIST(i, elmt, ls) {
		count += (int)isequal(&elmt, &v);
	}

	YASL_pushint(S, count);
	return 1;
}

enum SortType {
	SORT_TYPE_STR = -1,
	SORT_TYPE_EMPTY = 0,
	SORT_TYPE_NUM = 1
};


int custom_comp(struct YASL_State *S, struct YASL_Object a, struct YASL_Object b) {
	YASL_duptop(S);
	vm_push((struct VM *)S, a);
	vm_push((struct VM *)S, b);
	int num_returns = YASL_functioncall(S, 2);
	YASL_UNUSED(num_returns);
	if (!YASL_isbool(S)) {
		YASL_print_err(S, "TypeError: Expected a function returning bool, got %s.", YASL_peektypename(S));
		YASLX_throw_type_err(S);
	}
	bool a_lt_b = YASL_popbool(S);

	YASL_duptop(S);
	vm_push((struct VM *)S, b);
	vm_push((struct VM *)S, a);
	num_returns = YASL_functioncall(S, 2);
	YASL_UNUSED(num_returns);
	if (!YASL_isbool(S)) {
		YASL_print_err(S, "TypeError: Expected a function returning bool, got %s.", YASL_peektypename(S));
		YASLX_throw_type_err(S);
	}

	bool a_gt_b = YASL_popbool(S);

	if (a_lt_b == a_gt_b) return 0;
	return a_lt_b ? -1 : a_gt_b ? 1 : 0;
}

#define CUSTOM_COMP(a, b) custom_comp(S, a, b)

#define DEF_SORT(name, COMP) \
static void name##sort(struct YASL_State *S, struct YASL_Object *list, const size_t len) {\
	/* Base cases*/ \
	struct YASL_Object tmpObj;\
	if (len < 2) return;\
	if (len == 2) {\
		if (COMP(list[0], list[1]) > 0) {\
			tmpObj = list[0];\
			list[0] = list[1];\
			list[1] = tmpObj;\
		}\
		return;\
	}\
\
	/* YASL_Set sorting bounds */\
	size_t left = 0;\
	size_t right = len - 1;\
\
	/* Determine random midpoint to use (good average case) */\
	const size_t randIndex = rand() % len;\
	const struct YASL_Object mid = list[randIndex];\
\
	/* Determine exact number of items less than mid (mid's index)\
	   Furthermore, ensure list is not homogeneous to avoid infinite loops */\
	size_t ltCount = 0;\
	bool seenDifferent = false;\
	for (size_t i = 0; i < len; i++) {\
		if (COMP(list[i], mid) < 0) ltCount++;\
		if (seenDifferent == 0 && COMP(list[0], list[i]) != 0) seenDifferent = true;\
	}\
	if (!seenDifferent) return;\
\
	/* Ensure all items are on the correct side of mid */\
	while (left < right) {\
		while (COMP(list[left], mid) < 0) left++;\
		while (COMP(list[right], mid) >= 0) {\
			if (right == 0) break;\
			right--;\
		}\
\
		int cmp = COMP(list[left], list[right]);\
		if (cmp > 0 && left < right) {\
			tmpObj = list[right];\
			list[right] = list[left];\
			list[left++] = tmpObj;\
			if (right == 0) break;\
			right--;\
		} else if (cmp == 0) {\
			left++;\
			if (right == 0) break;\
			right--;\
		}\
	}\
\
	/* Let sort() finish that for us...*/ \
	name##sort(S, list, ltCount);\
	name##sort(S, &list[ltCount], len - ltCount);\
}

DEF_SORT(default, yasl_object_cmp)
// DEF_SORT(fn, CUSTOM_COMP)

// TODO: clean this up
int list_sort(struct YASL_State *S) {
	struct YASL_List *list = YASLX_checknlist(S, "list.sort", 0);

	/*
	if (!YASL_isundef(S)) {
		fnsort(S, list->items, list->count);
		return 0;
	}
	 */

	if (YASL_List_length(list) <= 1) {
		return 0;
	}

	enum SortType type = SORT_TYPE_EMPTY;

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
			vm_print_err_value((struct VM *)S, "%s expected a list of all numbers or all strings.", "list.sort");
			YASLX_throw_value_err(S);
		}
	}

	if (type != SORT_TYPE_EMPTY) {
		defaultsort(S, list->items, list->count);
	}

	return 0;
}

int list_insert(struct YASL_State *S) {
	struct YASL_Object value = vm_pop((struct VM *) S);
	yasl_int index = YASLX_checknint(S, "list.insert", 1);
	struct YASL_List *ls = YASLX_checknlist(S, "list.insert", 0);
	const yasl_int len = YASL_List_length(ls);

	if (index == len) {
		YASL_List_push(ls, value);
		YASL_pop(S);
		return 1;
	}

	if (index >= len || index < -len) {
		YASLX_print_value_err(S, "unable to insert item at index %" PRId64 " into list of length %" PRId64 ".", index, len);
		YASLX_throw_value_err(S);
	}

	if (index < 0) index += len;

	YASL_List_insert(ls, index, value);

	YASL_pop(S);
	return 1;
}
