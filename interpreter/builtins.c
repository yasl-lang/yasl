#include "builtins.h"

#include "data-structures/YASL_String.h"
#include "str_methods.h"
#include "undef_methods.h"
#include "float_methods.h"
#include "int_methods.h"
#include "bool_methods.h"
#include "table_methods.h"
#include "list_methods.h"
#include "VM.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                   *
 *                                                                                                                   *
 *                                                 VTABLES                                                           *
 *                                                                                                                   *
 *                                                                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void table_insert_specialstring_cfunction(struct VM *vm, struct YASL_Table *ht, int index, YASL_cfn addr, int num_args) {
	struct YASL_String *string = vm->special_strings[index];
	struct YASL_Object ko = YASL_STR(string), vo = YASL_CFN(addr, num_args);
	YASL_Table_insert_fast(ht, ko, vo);
}

struct YASL_Table *undef_builtins(struct VM *vm) {
	struct YASL_Table* table = YASL_Table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &undef_tostr, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOBOOL, &undef_tobool, 1);
	return table;
}

struct YASL_Table* float_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOINT, &float_toint, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOBOOL, &float_tobool, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOFLOAT, &float_tofloat, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &float_tostr, 1);
	return table;
}

struct YASL_Table* int_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOINT, &int_toint, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOBOOL, &int_tobool, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOFLOAT, &int_tofloat, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &int_tostr, 2);
	table_insert_specialstring_cfunction(vm, table, S_TOCHAR, &int_tochar, 1);
	return table;
}

struct YASL_Table* bool_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &bool_tostr, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOBOOL, &bool_tobool, 1);
	table_insert_specialstring_cfunction(vm, table, S___BOR, &bool___bor, 2);
	table_insert_specialstring_cfunction(vm, table, S___BAND, &bool___band, 2);
	table_insert_specialstring_cfunction(vm, table, S___BANDNOT, &bool___bandnot, 2);
	table_insert_specialstring_cfunction(vm, table, S___BXOR, &bool___bxor, 2);
	table_insert_specialstring_cfunction(vm, table, S___BNOT, &bool___bnot, 1);
	return table;
}

struct YASL_Table* str_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_specialstring_cfunction(vm, table, S___LEN, &str___len, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOFLOAT, &str_tofloat, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOINT, &str_toint, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISALNUM, &str_isalnum, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISAL, &str_isal, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISNUM, &str_isnum, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISSPACE, &str_isspace, 1);
	table_insert_specialstring_cfunction(vm, table, S_ISPRINT, &str_isprint, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOBOOL, &str_tobool, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &str_tostr, 2);
	table_insert_specialstring_cfunction(vm, table, S_TOLIST, &str_tolist, 2);
	table_insert_specialstring_cfunction(vm, table, S_SPREAD, &str_spread, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOUPPER, &str_toupper, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOLOWER, &str_tolower, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOBYTE, &str_tobyte, 1);
	table_insert_specialstring_cfunction(vm, table, S_STARTSWITH, &str_startswith, 2);
	table_insert_specialstring_cfunction(vm, table, S_ENDSWITH, &str_endswith, 2);
	table_insert_specialstring_cfunction(vm, table, S_REPLACE, &str_replace, 4);
	table_insert_specialstring_cfunction(vm, table, S_SEARCH, &str_search, 2);
	table_insert_specialstring_cfunction(vm, table, S_COUNT, &str_count, 2);
	table_insert_specialstring_cfunction(vm, table, S_SPLIT, &str_split, 3);
	table_insert_specialstring_cfunction(vm, table, S_LTRIM, &str_ltrim, 2);
	table_insert_specialstring_cfunction(vm, table, S_RTRIM, &str_rtrim, 2);
	table_insert_specialstring_cfunction(vm, table, S_TRIM, &str_trim, 2);
	table_insert_specialstring_cfunction(vm, table, S___GET, &str___get, 2);
	table_insert_specialstring_cfunction(vm, table, S_REP, &str_repeat, 2);
	table_insert_specialstring_cfunction(vm, table, S___ITER, &str___iter, 1);
	return table;
}

struct YASL_Table* list_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_specialstring_cfunction(vm, table, S___LEN, &list___len, 1);
	table_insert_specialstring_cfunction(vm, table, S_PUSH, &list_push, 2);
	table_insert_specialstring_cfunction(vm, table, S_COPY, &list_copy, 1);
	table_insert_specialstring_cfunction(vm, table, S___ADD, &list___add, 2);
	table_insert_specialstring_cfunction(vm, table, S___EQ, &list___eq, 2);
	// table_insert_specialstring_cfunction(vm, table, S_EXTEND, &list_extend, 2);
	table_insert_specialstring_cfunction(vm, table, S_POP, &list_pop, 1);
	table_insert_specialstring_cfunction(vm, table, S___GET, &list___get, 2);
	table_insert_specialstring_cfunction(vm, table, S___SET, &list___set, 3);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &list_tostr, 2);
	table_insert_specialstring_cfunction(vm, table, S_SEARCH, &list_search, 2);
	table_insert_specialstring_cfunction(vm, table, S_REVERSE, &list_reverse, 1);
	table_insert_specialstring_cfunction(vm, table, S_REMOVE, &list_remove, 2);
	table_insert_specialstring_cfunction(vm, table, S_CLEAR, &list_clear, 1);
	table_insert_specialstring_cfunction(vm, table, S_JOIN, &list_join, 2);
	table_insert_specialstring_cfunction(vm, table, S_SORT, &list_sort, 1);
	table_insert_specialstring_cfunction(vm, table, S_SPREAD, &list_spread, 1);
	table_insert_specialstring_cfunction(vm, table, S_COUNT, &list_count, 2);
	table_insert_specialstring_cfunction(vm, table, S_INSERT, &list_insert, 3);
	table_insert_specialstring_cfunction(vm, table, S___ITER, &list___iter, 1);
	return table;
}

struct YASL_Table* table_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_specialstring_cfunction(vm, table, S___LEN, &table___len, 1);
	table_insert_specialstring_cfunction(vm, table, S_REMOVE, &table_remove, 2);
	table_insert_specialstring_cfunction(vm, table, S_KEYS, &table_keys, 1);
	table_insert_specialstring_cfunction(vm, table, S_VALUES, &table_values, 1);
	table_insert_specialstring_cfunction(vm, table, S_COPY, &table_copy, 1);
	table_insert_specialstring_cfunction(vm, table, S_TOSTR, &table_tostr, 2);
	table_insert_specialstring_cfunction(vm, table, S___GET, &table___get, 2);
	table_insert_specialstring_cfunction(vm, table, S___SET, &table___set, 3);
	table_insert_specialstring_cfunction(vm, table, S___BOR, &table___bor, 2);
	table_insert_specialstring_cfunction(vm, table, S___EQ, &table___eq, 2);
	table_insert_specialstring_cfunction(vm, table, S_CLEAR, &table_clear, 1);
	table_insert_specialstring_cfunction(vm, table, S___ITER, &table___iter, 1);
	return table;
}
