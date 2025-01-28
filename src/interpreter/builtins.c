#include "builtins.h"

#include "data-structures/YASL_String.h"
#include "src/interpreter/methods/str_methods.h"
#include "src/interpreter/methods/undef_methods.h"
#include "src/interpreter/methods/float_methods.h"
#include "src/interpreter/methods/int_methods.h"
#include "src/interpreter/methods/bool_methods.h"
#include "src/interpreter/methods/table_methods.h"
#include "src/interpreter/methods/list_methods.h"
#include "VM.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                   *
 *                                                                                                                   *
 *                                                 METATABLES                                                        *
 *                                                                                                                   *
 *                                                                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct YASL_String *vm_lookup_interned_zstr(struct VM *vm, const char *chars);

void table_insert_str_cfunction(struct VM *vm, struct YASL_Table *ht, const char *name, YASL_cfn addr, int num_args) {
	struct YASL_String *string = vm_lookup_interned_zstr(vm, name);
	struct YASL_Object ko = YASL_STR(string), vo = YASL_CFN(addr, num_args);
	YASL_Table_insert_fast(ht, ko, vo);
}

struct YASL_Table *undef_builtins(struct VM *vm) {
	struct YASL_Table* table = YASL_Table_new();
	table_insert_str_cfunction(vm, table, "tostr", &undef_tostr, 1);
	table_insert_str_cfunction(vm, table, "tobool", &undef_tobool, 1);
	return table;
}

struct YASL_Table* float_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_str_cfunction(vm, table, "toint", &float_toint, 1);
	table_insert_str_cfunction(vm, table, "tobool", &float_tobool, 1);
	table_insert_str_cfunction(vm, table, "tofloat", &float_tofloat, 1);
	table_insert_str_cfunction(vm, table, "tostr", &float_tostr, 1);
	return table;
}

struct YASL_Table* int_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_str_cfunction(vm, table, "toint", &int_toint, 1);
	table_insert_str_cfunction(vm, table, "tobool", &int_tobool, 1);
	table_insert_str_cfunction(vm, table, "tofloat", &int_tofloat, 1);
	table_insert_str_cfunction(vm, table, "tostr", &int_tostr, 2);
	table_insert_str_cfunction(vm, table, "tochar", &int_tochar, 1);
	return table;
}

struct YASL_Table* bool_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_str_cfunction(vm, table, "tostr", &bool_tostr, 1);
	table_insert_str_cfunction(vm, table, "tobool", &bool_tobool, 1);
	table_insert_str_cfunction(vm, table, "__bor", &bool___bor, 2);
	table_insert_str_cfunction(vm, table, "__band", &bool___band, 2);
	table_insert_str_cfunction(vm, table, "__bandnot", &bool___bandnot, 2);
	table_insert_str_cfunction(vm, table, "__bxor", &bool___bxor, 2);
	table_insert_str_cfunction(vm, table, "__bnot", &bool___bnot, 1);
	return table;
}

struct YASL_Table* str_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_str_cfunction(vm, table, "__len", &str___len, 1);
	table_insert_str_cfunction(vm, table, "tobool", &str_tobool, 1);
	table_insert_str_cfunction(vm, table, "tostr", &str_tostr, 2);
	table_insert_str_cfunction(vm, table, "tolist", &str_tolist, 2);
	table_insert_str_cfunction(vm, table, "tofloat", &str_tofloat, 1);
	table_insert_str_cfunction(vm, table, "toint", &str_toint, 1);
	table_insert_str_cfunction(vm, table, "isalnum", &str_isalnum, 1);
	table_insert_str_cfunction(vm, table, "isal", &str_isal, 1);
	table_insert_str_cfunction(vm, table, "isnum", &str_isnum, 1);
	table_insert_str_cfunction(vm, table, "isspace", &str_isspace, 1);
	table_insert_str_cfunction(vm, table, "isprint", &str_isprint, 1);
	table_insert_str_cfunction(vm, table, "islower", &str_islower, 1);
	table_insert_str_cfunction(vm, table, "isupper", &str_isupper, 1);
	table_insert_str_cfunction(vm, table, "spread", &str_spread, 1);
	table_insert_str_cfunction(vm, table, "toupper", &str_toupper, 1);
	table_insert_str_cfunction(vm, table, "tolower", &str_tolower, 1);
	table_insert_str_cfunction(vm, table, "tobyte", &str_tobyte, 1);
	table_insert_str_cfunction(vm, table, "startswith", &str_startswith, 2);
	table_insert_str_cfunction(vm, table, "endswith", &str_endswith, 2);
	table_insert_str_cfunction(vm, table, "replace", &str_replace, 4);
	table_insert_str_cfunction(vm, table, "search", &str_search, 3);
	table_insert_str_cfunction(vm, table, "count", &str_count, 2);
	table_insert_str_cfunction(vm, table, "split", &str_split, 3);
	table_insert_str_cfunction(vm, table, "partition", &str_partition, -2);
	table_insert_str_cfunction(vm, table, "ltrim", &str_ltrim, 2);
	table_insert_str_cfunction(vm, table, "rtrim", &str_rtrim, 2);
	table_insert_str_cfunction(vm, table, "trim", &str_trim, 2);
	table_insert_str_cfunction(vm, table, "rep", &str_repeat, 2);
	table_insert_str_cfunction(vm, table, "has", &str_has, 3);
	table_insert_str_cfunction(vm, table, "__get", &str___get, 2);
	table_insert_str_cfunction(vm, table, "__iter", &str___iter, 1);
	return table;
}

struct YASL_Table* list_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_str_cfunction(vm, table, "__len", &list___len, 1);
	table_insert_str_cfunction(vm, table, "push", &list_push, 2);
	table_insert_str_cfunction(vm, table, "copy", &list_copy, 1);
	table_insert_str_cfunction(vm, table, "__add", &list___add, 2);
	table_insert_str_cfunction(vm, table, "__eq", &list___eq, 2);
	// table_insert_str_cfunction(vm, table, "extend", &list_extend, 2);
	table_insert_str_cfunction(vm, table, "pop", &list_pop, 1);
	table_insert_str_cfunction(vm, table, "__get", &list___get, 2);
	table_insert_str_cfunction(vm, table, "__set", &list___set, 3);
	table_insert_str_cfunction(vm, table, "tostr", &list_tostr, 2);
	table_insert_str_cfunction(vm, table, "search", &list_search, 3);
	table_insert_str_cfunction(vm, table, "reverse", &list_reverse, 1);
	table_insert_str_cfunction(vm, table, "remove", &list_remove, 2);
	table_insert_str_cfunction(vm, table, "clear", &list_clear, 1);
	table_insert_str_cfunction(vm, table, "join", &list_join, 2);
	table_insert_str_cfunction(vm, table, "sort", &list_sort, 2);
	table_insert_str_cfunction(vm, table, "spread", &list_spread, 1);
	table_insert_str_cfunction(vm, table, "count", &list_count, 2);
	table_insert_str_cfunction(vm, table, "insert", &list_insert, 3);
	table_insert_str_cfunction(vm, table, "shuffle", &list_shuffle, 1);
	table_insert_str_cfunction(vm, table, "has", &list_has, 3);
	table_insert_str_cfunction(vm, table, "__iter", &list___iter, 1);
	return table;
}

struct YASL_Table* table_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
	table_insert_str_cfunction(vm, table, "__len", &table___len, 1);
	table_insert_str_cfunction(vm, table, "remove", &table_remove, 2);
	table_insert_str_cfunction(vm, table, "keys", &table_keys, 1);
	table_insert_str_cfunction(vm, table, "values", &table_values, 1);
	table_insert_str_cfunction(vm, table, "copy", &table_copy, 1);
	table_insert_str_cfunction(vm, table, "tostr", &table_tostr, 2);
	table_insert_str_cfunction(vm, table, "__get", &table___get, 2);
	table_insert_str_cfunction(vm, table, "__set", &table___set, 3);
	table_insert_str_cfunction(vm, table, "__bor", &table___bor, 2);
	table_insert_str_cfunction(vm, table, "__eq", &table___eq, 2);
	table_insert_str_cfunction(vm, table, "clear", &table_clear, 1);
	table_insert_str_cfunction(vm, table, "__iter", &table___iter, 1);
	return table;
}
