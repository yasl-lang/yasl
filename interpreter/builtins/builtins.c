#include "builtins.h"

#include "undef_methods.h"
#include "str_methods.h"
#include "float_methods.h"
#include "int_methods.h"
#include "bool_methods.h"
#include "table_methods.h"
#include "list_methods.h"
#include "VM.h"

void yasl_print(struct VM* vm) {
	if (!YASL_ISSTR(VM_PEEK(vm, vm->sp))) {
		YASL_Types index = vm_peek(vm).type;
		struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
		struct YASL_Object *result = table_search(vm->builtins_htable[index], key);
		str_del(YASL_GETSTR(key));
		YASL_GETCFN(*result)->value((struct YASL_State *) &vm);
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

struct Table *undef_builtins() {
	struct Table* table = table_new();
	table_insert_literalcstring_cfunction(table, "tostr", &undef_tostr, 1);
	return table;
}

struct Table* float_builtins() {
	struct Table *table = table_new();
	table_insert_literalcstring_cfunction(table, "toint", &float_toint, 1);
	table_insert_literalcstring_cfunction(table, "tostr", &float_tostr, 1);
	return table;
}


struct Table* int_builtins() {
	struct Table *table = table_new();
	table_insert_literalcstring_cfunction(table, "tofloat", &int_tofloat, 1);
	table_insert_literalcstring_cfunction(table, "tostr", &int_tostr, 1);
	return table;
}

struct Table* bool_builtins() {
	struct Table *table = table_new();
	table_insert_literalcstring_cfunction(table, "tostr", &bool_tostr, 1);
	return table;
}


struct Table* str_builtins() {
	struct Table *table = table_new();
	table_insert_literalcstring_cfunction(table, "tofloat", &str_tofloat, 1);
	table_insert_literalcstring_cfunction(table, "toint", &str_toint, 1);
	table_insert_literalcstring_cfunction(table, "isalnum", &str_isalnum, 1);
	table_insert_literalcstring_cfunction(table, "isal", &str_isal, 1);
	table_insert_literalcstring_cfunction(table, "isnum", &str_isnum, 1);
	table_insert_literalcstring_cfunction(table, "isspace", &str_isspace, 1);
	table_insert_literalcstring_cfunction(table, "tobool", &str_tobool, 1);
	table_insert_literalcstring_cfunction(table, "tostr", &str_tostr, 1);
	table_insert_literalcstring_cfunction(table, "toupper", &str_toupper, 1);
	table_insert_literalcstring_cfunction(table, "tolower", &str_tolower, 1);
	table_insert_literalcstring_cfunction(table, "startswith", &str_startswith, 2);
	table_insert_literalcstring_cfunction(table, "endswith", &str_endswith, 2);
	table_insert_literalcstring_cfunction(table, "replace", &str_replace, 3);
	table_insert_literalcstring_cfunction(table, "search", &str_search, 2);
	table_insert_literalcstring_cfunction(table, "slice", &str_slice, 3);
	table_insert_literalcstring_cfunction(table, "split", &str_split, 2);
	table_insert_literalcstring_cfunction(table, "ltrim", &str_ltrim, 2);
	table_insert_literalcstring_cfunction(table, "rtrim", &str_rtrim, 2);
	table_insert_literalcstring_cfunction(table, "trim", &str_trim, 2);
	table_insert_literalcstring_cfunction(table, "__get", &str___get, 2);
	table_insert_literalcstring_cfunction(table, "repeat", &str_repeat, 2);
	return table;
}

struct Table* list_builtins() {
	struct Table *table = table_new();
	table_insert_literalcstring_cfunction(table, "push", &list_push, 2);
	table_insert_literalcstring_cfunction(table, "copy", &list_copy, 1);
	table_insert_literalcstring_cfunction(table, "extend", &list_extend, 2);
	table_insert_literalcstring_cfunction(table, "pop", &list_pop, 1);
	table_insert_literalcstring_cfunction(table, "__get", &list___get, 2);
	table_insert_literalcstring_cfunction(table, "__set", &list___set, 3);
	table_insert_literalcstring_cfunction(table, "tostr", &list_tostr, 1);
	table_insert_literalcstring_cfunction(table, "search", &list_search, 2);
	table_insert_literalcstring_cfunction(table, "reverse", &list_reverse, 1);
	table_insert_literalcstring_cfunction(table, "slice", &list_slice, 3);
	table_insert_literalcstring_cfunction(table, "clear", &list_clear, 1);
	table_insert_literalcstring_cfunction(table, "join", &list_join, 2);
	return table;
}

struct Table* table_builtins() {
	struct Table *table = table_new();
	table_insert_literalcstring_cfunction(table, "keys", &table_keys, 1);
	table_insert_literalcstring_cfunction(table, "values", &table_values, 1);
	table_insert_literalcstring_cfunction(table, "copy", &table_clone, 1);
	table_insert_literalcstring_cfunction(table, "tostr", &table_tostr, 1);
	table_insert_literalcstring_cfunction(table, "__get", &table___get, 2);
	table_insert_literalcstring_cfunction(table, "__set", &table___set, 3);
	table_insert_literalcstring_cfunction(table, "clear", &table_clear, 1);
	return table;
}

