#include <interpreter/undef/undef_methods.h>
#include "builtins.h"

#include "interpreter/YASL_string/str_methods.h"
#include "interpreter/float/float_methods.h"
#include "interpreter/integer/int_methods.h"
#include "interpreter/boolean/bool_methods.h"
#include "interpreter/table/table_methods.h"
#include "interpreter/list/list_methods.h"
#include "interpreter/VM/VM.h"

void yasl_print(struct VM* vm) {
	if (!YASL_ISSTR(VM_PEEK(vm, vm->sp))) {
		YASL_Types index = vm_peek(vm).type;
		struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
		struct YASL_Object result = table_search(vm->builtins_htable[index], key);
		str_del(YASL_GETSTR(key));
		YASL_GETCFN(result)->value((struct YASL_State *) vm);
	}
	struct YASL_Object v = vm_pop(vm);
	for (int64_t i = 0; i < yasl_string_len(YASL_GETSTR(v)); i++) {
		printf("%c", YASL_GETSTR(v)->str[i + YASL_GETSTR(v)->start]);
	}
	printf("\n");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                   *
 *                                                                                                                   *
 *                                                 VTABLES                                                           *
 *                                                                                                                   *
 *                                                                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void table_insert_specialstring_cfunction(struct VM *vm, struct Table *ht, int index, int (*addr)(struct YASL_State *), int num_args) {
	String_t *string = vm->special_strings[index];
	table_insert(ht, YASL_STR(string), YASL_CFN(addr, num_args));
}

struct Table *undef_builtins(struct VM *vm) {
	struct Table* table = table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &undef_tostr, 1);
	return table;
}

struct Table* float_builtins(struct VM *vm) {
	struct Table *table = table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOINT, &float_toint, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOFLOAT, &float_tofloat, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &float_tostr, 1);
	return table;
}


struct Table* int_builtins(struct VM *vm) {
	struct Table *table = table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOINT, &int_toint, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOFLOAT, &int_tofloat, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &int_tostr, 1);
	return table;
}

struct Table* bool_builtins(struct VM *vm) {
	struct Table *table = table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &bool_tostr, 1);
	return table;
}


struct Table* str_builtins(struct VM *vm) {
	struct Table *table = table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOFLOAT, &str_tofloat, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOINT, &str_toint, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISALNUM, &str_isalnum, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISAL, &str_isal, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISNUM, &str_isnum, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISSPACE, &str_isspace, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOBOOL, &str_tobool, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &str_tostr, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOUPPER, &str_toupper, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOLOWER, &str_tolower, 1);
	table_insert_specialstring_cfunction(vm, table, S_STARTSWITH, &str_startswith, 2);
	table_insert_specialstring_cfunction(vm, table, S_ENDSWITH, &str_endswith, 2);
	table_insert_specialstring_cfunction(vm, table, S_REPLACE, &str_replace, 3);
	table_insert_specialstring_cfunction(vm, table, S_SEARCH, &str_search, 2);
	table_insert_specialstring_cfunction(vm, table, S_COUNT, &str_count, 2);
	table_insert_specialstring_cfunction(vm, table, S_SLICE, &str_slice, 3);
	table_insert_specialstring_cfunction(vm, table, S_SPLIT, &str_split, 2);
	table_insert_specialstring_cfunction(vm, table, S_LTRIM, &str_ltrim, 2);
	table_insert_specialstring_cfunction(vm, table, S_RTRIM, &str_rtrim, 2);
	table_insert_specialstring_cfunction(vm, table, S_TRIM, &str_trim, 2);
	table_insert_specialstring_cfunction(vm, table, S___GET, &str___get, 2);
	table_insert_specialstring_cfunction(vm, table, S_REPEAT, &str_repeat, 2);
	return table;
}

struct Table* list_builtins(struct VM *vm) {
	struct Table *table = table_new();
	table_insert_specialstring_cfunction(vm, table, S_PUSH, &list_push, 2);
	table_insert_specialstring_cfunction(vm, table, S_COPY, &list_copy, 1);
	table_insert_specialstring_cfunction(vm, table, S_EXTEND, &list_extend, 2);
	table_insert_specialstring_cfunction(vm, table, S_POP, &list_pop, 1);
	table_insert_specialstring_cfunction(vm, table, S___GET, &list___get, 2);
	table_insert_specialstring_cfunction(vm, table, S___SET, &list___set, 3);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &list_tostr, 1);
	table_insert_specialstring_cfunction(vm, table, S_SEARCH, &list_search, 2);
	table_insert_specialstring_cfunction(vm, table, S_REVERSE, &list_reverse, 1);
	table_insert_specialstring_cfunction(vm, table, S_SLICE, &list_slice, 3);
	table_insert_specialstring_cfunction(vm, table, S_CLEAR, &list_clear, 1);
	table_insert_specialstring_cfunction(vm, table, S_JOIN, &list_join, 2);
	table_insert_specialstring_cfunction(vm, table, S_SORT, &list_sort, 1);
	return table;
}

struct Table* table_builtins(struct VM *vm) {
	struct Table *table = table_new();
	table_insert_specialstring_cfunction(vm, table, S_KEYS, &table_keys, 1);
	table_insert_specialstring_cfunction(vm, table, S_VALUES, &table_values, 1);
	table_insert_specialstring_cfunction(vm, table, S_COPY, &table_clone, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &table_tostr, 1);
	table_insert_specialstring_cfunction(vm, table, S___GET, &table___get, 2);
	table_insert_specialstring_cfunction(vm, table, S___SET, &table___set, 3);
	table_insert_specialstring_cfunction(vm, table, S_CLEAR, &table_clear, 1);
	return table;
}
