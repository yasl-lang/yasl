#include <src/interpreter/YASL_Object.h>
#include "list_methods.h"

#include <ctype.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "data-structures/YASL_List.h"
#include "util/hash_function.h"
#include "yasl_error.h"
#include "yasl_state.h"

static struct YASL_List *YASLX_checknlist(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnlist(S, pos)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, pos, YASL_LIST_NAME);
	}
	return (struct YASL_List *)YASL_peeknuserdata(S, pos);
}

void list___get_helper(struct YASL_State *S, struct YASL_List *ls, yasl_int index) {
	if (index < -(int64_t) ls->count || index >= (int64_t) ls->count) {
		YASLX_print_and_throw_err_value(S, "unable to index list of length %" PRI_SIZET " with index %" "ld" ".", ls->count, (long)index);
	} else {
		if (index >= 0) {
			vm_push((struct VM *) S, ls->items[index]);
		} else {
			vm_push((struct VM *) S, ls->items[index + ls->count]);
		}
	}
}

int list___get(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.__get", 0);
	yasl_int index = YASLX_checknint(S, "list.__get", 1);

	list___get_helper(S, ls, index);
	return 1;
}

int list___len(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.__len", 0);
	YASL_pushint(S, YASL_List_len(ls));
	return 1;
}

int list___set(struct YASL_State *S) {
	struct YASL_Object value = vm_pop((struct VM *) S);
	struct YASL_List *ls = YASLX_checknlist(S, "list.__set", 0);
	yasl_int index = YASLX_checknint(S, "list.__set", 1);

	if (index < -(yasl_int) ls->count || index >= (yasl_int) ls->count) {
		YASLX_print_and_throw_err_value(S, "unable to index list of length %" PRI_SIZET " with index %" PRId64 ".", ls->count, index);
	}

	if (index < 0) index += ls->count;

	inc_ref(&value);
	vm_dec_ref(&S->vm, ls->items + index);
	ls->items[index] = value;
	return 1;
}

static int list___next(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.__next", 0);
	yasl_int curr = YASLX_checknint(S, "list.__next", 1);

	if (curr < 0 || curr >= (yasl_int)ls->count) {
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
	struct RC_UserData *new_ls = rcls_new_sized(&S->vm, ls->size);
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
	struct RC_UserData *ptr = rcls_new_sized(&S->vm, size);
	for (size_t i = 0; i < a->count; i++) {
		YASL_List_push((struct YASL_List *) ptr->data, (a)->items[i]);
	}
	for (size_t i = 0; i < (b)->count; i++) {
		YASL_List_push((struct YASL_List *) ptr->data, (b)->items[i]);
	}

	return ptr;
}

int list___add(struct YASL_State *S) {
	struct YASL_List *a = YASLX_checknlist(S, "list.__add", 0);
	struct YASL_List *b = YASLX_checknlist(S, "list.__add", 1);

	vm_pushlist((struct VM *) S, list_concat(S, a, b));
	return 1;
}

int list___eq(struct YASL_State *S) {
	struct YASL_List *left = YASLX_checknlist(S, "list.__eq", 0);
	struct YASL_List *right = YASLX_checknlist(S, "list.__eq", 1);

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
		YASLX_print_and_throw_err_value(S, "%s expected nonempty list as arg 0.", "list.pop");
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
			size_t remaining = ls->count - i - 1;
			memmove(ls->items + i, ls->items + i + 1, remaining * sizeof(struct YASL_Object));
			ls->count--;
			break;
		}
	}

	YASL_pop(S);
	return 1;
}

int list_removeindex(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.removeindex", 0);
	yasl_int index = YASLX_checknint(S, "list.removeindex", 1);

	if (index < 0 || index >= (yasl_int)ls->count) {
		YASLX_print_and_throw_err_value(S, "list.removeindex expected a value between 0 and %" PRI_SIZET ", got %" PRId64, YASL_List_len(ls), index);
	}

	// decrement refcount for the removed element
	vm_dec_ref(&S->vm, &ls->items[index]);

	// shift elements left to fill the gap
	size_t remaining = ls->count - index - 1;
	if (remaining > 0) {
		memmove(ls->items + index,
			ls->items + index + 1,
			remaining * sizeof(struct YASL_Object));
	}

	ls->count--;

	// remove the index argument from the stack (keep the list there)
	YASL_pop(S);

	return 1;
}

int list_search(struct YASL_State *S) {
	yasl_int start = YASLX_checknoptint(S, "list.search", 2, 0);
	YASL_pop(S);
	struct YASL_Object needle = vm_pop((struct VM *) S);
	struct YASL_List *haystack = YASLX_checknlist(S, "list.search", 0);
	struct YASL_Object index = YASL_UNDEF();

	const yasl_int len = (yasl_int)YASL_List_len(haystack);
	if (start < 0 || start > len) {
		YASLX_print_and_throw_err_value(S, "list.search expected a starting index between 0 and %" PRI_SIZET ", got %" PRId64, (size_t)len, start);
	}

	FOR_LIST_START(i, obj, haystack, start) {
		if ((isequal(&obj, &needle))) {
			index = YASL_INT((yasl_int) i);
			break;
		}
	}

	vm_push((struct VM *) S, index);
	return 1;
}

int list_searchall(struct YASL_State *S) {
	yasl_int start = YASLX_checknoptint(S, "list.searchall", 2, 0);
	YASL_pop(S);   // pop start
	struct YASL_Object needle = vm_pop((struct VM *) S);
	struct YASL_List *haystack = YASLX_checknlist(S, "list.searchall", 0);

	yasl_int list_len = YASL_List_len(haystack);
	if (start < 0 || start > list_len) {
		YASLX_print_and_throw_err_value(S,
			"list.searchall expected a starting index between 0 and %" PRI_SIZET ", got %" PRId64,
			(size_t)list_len, start);
	}

	// create result list
	struct RC_UserData *list_ud = rcls_new((struct VM *)S);
	struct YASL_List *results = (struct YASL_List*)list_ud->data;
	vm_pushlist((struct VM *) S, list_ud);

	// scan through haystack from start
	FOR_LIST_START(i, obj, haystack, start) {
		if (isequal(&obj, &needle)) {
			YASL_List_push(results, YASL_INT((yasl_int)i));
		}
	}

	// if no matches, return undef
	if (YASL_List_len(results) == 0) {
		YASL_pushundef(S);
		return 1;
	}

	vm_pushlist((struct VM *) S, list_ud);
	return 1;
}

int list_has(struct YASL_State *S) {
	yasl_int start = YASLX_checknoptint(S, "list.has", 2, 0);
	YASL_pop(S);
	struct YASL_Object needle = vm_pop((struct VM *) S);
	struct YASL_List *haystack = YASLX_checknlist(S, "list.has", 0);
	bool has = false;

	if (start < 0 || start > (yasl_int)YASL_List_len(haystack)) {
		YASLX_print_and_throw_err_value(S, "list.has expected a starting index between 0 and %" PRI_SIZET ", got %" PRId64, YASL_List_len(haystack), start);
	}

	FOR_LIST_START(i, obj, haystack, start) {
		if ((isequal(&obj, &needle))) {
			has = true;
			break;
		}
	}

	YASL_pushbool(S, has);
	return 1;
}

void table_tostr_helper(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format);
bool buffer_contains(BUFFER(ptr) buffer, void *val);
void rec_call(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format, void (*f)(struct YASL_State *, BUFFER(ptr), struct YASL_Object *));

#define FOUND_LIST "[...], "
#define FOUND_TABLE "{...}, "

void list_tostr_helper(struct YASL_State *S, BUFFER(ptr) buffer, struct YASL_Object *format) {
	struct YASL_List *list = vm_peeklist((struct VM *) S);
	if (list->count == 0) {
		YASL_pop(S);
		YASL_pushlit(S, "[]");
		return;
	}

	YASL_ByteBuffer bb = NEW_BB(8);

	YASL_ByteBuffer_add_byte(&bb, '[');

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

	vm_pushstr_bb((struct VM *)S, &bb);
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
	size_t len;
	const char *chars = YASLX_checknoptstrz(S, "list.join", 1, &len, "");
	struct YASL_List *list = YASLX_checknlist(S, "list.join", 0);

	if (list->count == 0) {
		vm_pushstr((struct VM *) S, YASL_String_new_copyz((struct VM *)S, ""));
		return 1;
	}

	YASL_ByteBuffer bb = NEW_BB(8);

	vm_push((struct VM *)S, list->items[0]);
	vm_stringify_top((struct VM *)S);
	struct YASL_String *str = vm_popstr((struct VM *) S);

	YASL_ByteBuffer_extend(&bb, (const byte *)YASL_String_chars(str), YASL_String_len(str));

	for (size_t i = 1; i < list->count; i++) {
		YASL_ByteBuffer_extend(&bb, (const byte *)chars, len);

		vm_push((struct VM *) S, list->items[i]);
		vm_stringify_top((struct VM *)S);
		struct YASL_String *str = vm_popstr((struct VM *) S);

		YASL_ByteBuffer_extend(&bb, (const byte *)YASL_String_chars(str), YASL_String_len(str));
	}

	vm_pushstr_bb((struct VM *)S, &bb);
	return 1;
}

int list_spread(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.spread", 0);
	const yasl_int len = YASL_List_len(ls);

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

int list_shuffle(struct YASL_State *S) {
	struct YASL_List *ls = YASLX_checknlist(S, "list.shuffle", 0);
	const size_t len = ls->count;

	if (len <= 1) return 1;

	// We use a Fisher-Yate shuffle here.
	for (size_t i = len - 1; i >= 1; i--) {
		size_t j = (size_t)ya_rand();
		j %= i + 1;
		YASL_ASSERT(j <= i, "j should be in this range.");
		struct YASL_Object tmp = ls->items[i];
		ls->items[i] = ls->items[j];
		ls->items[j] = tmp;
	}
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
		YASLX_throw_err_type(S);
	}
	bool a_lt_b = YASL_popbool(S);

	YASL_duptop(S);
	vm_push((struct VM *)S, b);
	vm_push((struct VM *)S, a);
	num_returns = YASL_functioncall(S, 2);
	YASL_UNUSED(num_returns);
	if (!YASL_isbool(S)) {
		YASL_print_err(S, "TypeError: Expected a function returning bool, got %s.", YASL_peektypename(S));
		YASLX_throw_err_type(S);
	}

	bool a_gt_b = YASL_popbool(S);

	if (a_lt_b == a_gt_b) return 0;
	return a_lt_b ? -1 : a_gt_b ? 1 : 0;
}
static struct YASL_Object transform(struct YASL_State *S, struct YASL_Object k) {
	struct YASL_String *s = k.value.sval;
	//s->s.str;
	//s->s.len;
	size_t curr = 0;
	struct RC_UserData *result = rcls_new(&S->vm);
	struct YASL_List *ls = (struct YASL_List *)result->data;
	size_t i = 0;
	for (; i < s->s.len; i++) {
		if (isdigit(s->s.str[i])) {
			if (curr != i) {
				YASL_List_push(ls, YASL_STR(YASL_String_new_substring(&S->vm, s, curr, i)));
				//printf("BA: `%*s`\n", (int)(i - curr), s->s.str + curr);
				curr = i;
			}

			while (isdigit(s->s.str[i])) {
				i++;
			}
			yasl_int n = YASL_String_toint(s->s.str + curr, i - curr);
			//printf("AB: `%*s`\n", (int)(i - curr), s->s.str + curr);
			YASL_List_push(ls, YASL_INT(n));
			curr = i;
		}
	}

	if (curr < s->s.len && curr < i) {
		// printf("CC: `%*s` (%zd, %zd)\n", (int)(i - curr), s->s.str + curr, i, curr);
		YASL_List_push(ls, YASL_STR(YASL_String_new_substring(&S->vm, s, curr, i)));
	}

	// printf("s: `%*s`, len: %d\n", (int)s->s.len, s->s.str, (int)ls->count);
	return YASL_LIST(result);
}

static struct YASL_List *search_or_add(struct YASL_State *S, struct YASL_Table *const vars, struct YASL_Object key) {
	struct YASL_Object val = YASL_Table_search(vars, key);
	if (val.type == Y_END) {
		val = transform(S, key);
		YASL_Table_insert(vars, key, val);
	}
	return (struct YASL_List *)(val.value.uval->data);
}

int natural_comp(struct YASL_State *S, struct YASL_Table *const vars, struct YASL_Object a, struct YASL_Object b) {
	YASL_UNUSED(S);
	if (isequal(&a, &b)) {
		return 0;
	}

	const struct YASL_List *a2 = search_or_add(S, vars, a);
	const struct YASL_List *b2 = search_or_add(S, vars, b);

	const size_t small = a2->count < b2->count ? a2->count : b2->count;
	for (size_t i = 0; i < small; i++) {
		struct YASL_Object a = a2->items[i];
		struct YASL_Object b = b2->items[i];
		if (a.type == Y_STR && b.type == Y_INT) {
			return -1;
		} else if (b.type == Y_STR && a.type == Y_INT) {
			return 1;
		}
		int curr = yasl_object_cmp(a, b);
		if (curr != 0) {
			return curr;
		}
	}

	return ((int)a2->count - (int)b2->count);
}

#define YASL_OBJ_COMP_REVERSE(a, b) (-yasl_object_cmp(a, b))
#define CUSTOM_COMP(a, b) custom_comp(S, a, b)
#define CUSTOM_COMP_REVERSE(a, b) (-custom_comp(S, a, b))
#define NATURAL_COMP(a, b) natural_comp(S, vars, a, b)

#define DEF_SORT(name, COMP) \
static void name##sort(struct YASL_State *S, struct YASL_Object *list, const size_t len, struct YASL_Table *const vars) {\
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
	const size_t randIndex = ya_rand() % len;\
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
	name##sort(S, list, ltCount, vars);\
	name##sort(S, &list[ltCount], len - ltCount, vars);\
}

DEF_SORT(default, yasl_object_cmp)
// DEF_SORT(reverse, YASL_OBJ_COMP_REVERSE)
DEF_SORT(fn, CUSTOM_COMP)
// DEF_SORT(fn_reverse, CUSTOM_COMP_REVERSE)
DEF_SORT(natural, NATURAL_COMP)

// TODO: clean this up
int list_sort(struct YASL_State *S) {
	struct YASL_List *list = YASLX_checknlist(S, "list.sort", 0);

	if (YASL_isstr(S)) {
		char *tmp = YASL_peekcstr(S);
		if (strcmp(tmp, "n") != 0) {
			YASLX_print_err_value(S, "Unknown option: %*s", (int)strlen(tmp), tmp);
			free(tmp);
			YASLX_throw_err_value(S);
		}
		free(tmp);
		for (size_t i = 0; i < list->count; i++) {
			if (list->items[i].type != Y_STR) {
				YASLX_print_and_throw_err_value(S, "%s expected a list of all numbers or all strings.",
								"list.sort");
			}
		}
		struct YASL_Table *vars = YASL_Table_new();
		naturalsort(S, list->items, list->count, vars);
		YASL_Table_del(vars);
		return 0;
	}

	if (!YASL_isnundef(S, 1)) {
		/*
		 * We save the old value of list before calling the custom sort function, so that the sort function
		 * modifying the list doesn't cause a segfault. To the custom sort function, it just sees an empty list.
		 */
		const struct YASL_List tmp = *list;
		*list = (struct YASL_List) { 0, 0, NULL };
		fnsort(S, tmp.items, tmp.count, NULL);
		if (list->items) YASL_List_del_data(S, list->items);
		*list = tmp;
		return 0;
	}

	if (YASL_List_len(list) <= 1) {
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
			YASLX_print_and_throw_err_value(S, "%s expected a list of all numbers or all strings.", "list.sort");
		}
	}

	if (type != SORT_TYPE_EMPTY) {
		defaultsort(S, list->items, list->count, NULL);
	}

	return 0;
}

int list_insert(struct YASL_State *S) {
	struct YASL_Object value = vm_pop((struct VM *) S);
	yasl_int index = YASLX_checknint(S, "list.insert", 1);
	struct YASL_List *ls = YASLX_checknlist(S, "list.insert", 0);
	const yasl_int len = YASL_List_len(ls);

	if (index == len) {
		YASL_List_push(ls, value);
		YASL_pop(S);
		return 1;
	}

	if (index >= len || index < -len) {
		YASLX_print_and_throw_err_value(S, "unable to insert item at index %" PRId64 " into list of length %" PRId64 ".", index, len);
	}

	if (index < 0) index += len;

	YASL_List_insert(ls, index, value);

	YASL_pop(S);
	return 1;
}
