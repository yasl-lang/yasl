#include "builtins.h"

#include "str_methods.h"
#include "undef_methods.h"
#include "float_methods.h"
#include "int_methods.h"
#include "bool_methods.h"
#include "table_methods.h"
#include "list_methods.h"
#include "VM.h"
#include "YASL_string.h"

void yasl_print(struct VM* vm) {
	vm_stringify_top(vm);
	String_t *v = vm_popstr(vm);
	printf("%.*s\n", (int)yasl_string_len(v), v->start + v->str);
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
	struct YASL_Object ko = YASL_STR(string), vo = YASL_CFN(addr, num_args);
	table_insert(ht, ko, vo);
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
	table_insert_specialstring_cfunction(vm, table, S_TOBOOL, &bool_tobool, 1);
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
	table_insert_specialstring_cfunction(vm, table, S_REP, &str_repeat, 2);
	return table;
}

struct Table* list_builtins(struct VM *vm) {
	struct Table *table = table_new();
	table_insert_specialstring_cfunction(vm, table, S_PUSH, &list_push, 2);
	table_insert_specialstring_cfunction(vm, table, S_COPY, &list_copy, 1);
	table_insert_specialstring_cfunction(vm, table, S___ADD, &list___add, 2);
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
	table_insert_specialstring_cfunction(vm, table, S_REMOVE, &table_remove, 2);
	table_insert_specialstring_cfunction(vm, table, S_KEYS, &table_keys, 1);
	table_insert_specialstring_cfunction(vm, table, S_VALUES, &table_values, 1);
	table_insert_specialstring_cfunction(vm, table, S_COPY, &table_clone, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &table_tostr, 1);
	table_insert_specialstring_cfunction(vm, table, S___GET, &table___get, 2);
	table_insert_specialstring_cfunction(vm, table, S___SET, &table___set, 3);
	table_insert_specialstring_cfunction(vm, table, S_CLEAR, &table_clear, 1);
	return table;
}
