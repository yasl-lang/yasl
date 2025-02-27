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

// \ttable_insert_str_cfunction\(vm, table, "[^"]*", &table_
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
#define X(name, arity) table_insert_str_cfunction(vm, table, #name, &bool_##name, arity);
#include "methods/bool_methods.x"
#undef X
	return table;
}

struct YASL_Table* str_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
#define X(name, arity) table_insert_str_cfunction(vm, table, #name, &str_##name, arity);
#include "methods/str_methods.x"
#undef X
	return table;
}

struct YASL_Table* list_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
#define X(name, arity) table_insert_str_cfunction(vm, table, #name, &list_##name, arity);
#include "methods/list_methods.x"
#undef X
	return table;
}

struct YASL_Table* table_builtins(struct VM *vm) {
	struct YASL_Table *table = YASL_Table_new();
#define X(name, arity) table_insert_str_cfunction(vm, table, #name, &table_##name, arity);
#include "methods/table_methods.x"
#undef X
	return table;
}
