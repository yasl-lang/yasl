#include "yasl-std-mt.h"

#include "yasl_aux.h"
#include "yasl_state.h"

int YASL_mt_get(struct YASL_State *S) {
	if (!YASL_isntable(S, 0) && !YASL_isnlist(S, 0) && !vm_isuserdata(&S->vm)) {
		vm_print_err_type((struct VM *)S, "cannot get metatable for value of type %s.", YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	vm_get_metatable((struct VM *)S);
	return 1;
}

int YASL_mt_set(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		YASLX_print_err_bad_arg_type(S, "mt.set", 1, YASL_TABLE_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	struct YASL_Object mt = vm_pop((struct VM *)S);
	switch (YASL_peektype(S)) {
	case Y_USERDATA:
	case Y_LIST:
	case Y_TABLE:
		ud_setmt(&S->vm, YASL_GETUSERDATA(vm_peek((struct VM *)S)), YASL_GETUSERDATA(mt));
		break;
	default:
		vm_print_err_type((struct VM *)S, "cannot set metatable for value of type %s.", YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return 1;
}

int YASL_mt_setself(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		YASLX_print_err_bad_arg_type(S, "mt.set", 1, YASL_TABLE_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	YASL_duptop(S);
	return YASL_mt_set(S);
}

int YASL_mt_lookup(struct YASL_State *S) {
	struct YASL_Object key = vm_pop((struct VM *)S);
	vm_get_metatable((struct VM *)S);

	if (YASL_isundef(S)) {
		return 1;
	}

	struct YASL_Table* table = vm_poptable((struct VM *)S);
	int result = vm_lookup_method_helper((struct VM *)S, table, key);

	if (result != YASL_SUCCESS) {
		YASL_pushundef(S);
		return 1;
	}

	return 1;
}

int YASL_decllib_mt(struct YASL_State *S) {
	YASL_declglobal(S, "mt");
	YASL_pushtable(S);
	YASL_setglobal(S, "mt");

	YASL_loadglobal(S, "mt");

	struct YASLX_function functions[] = {
		{"get",     YASL_mt_get,     1},
		{"set",     YASL_mt_set,     2},
		{"setself", YASL_mt_setself, 1},
		{"lookup",  YASL_mt_lookup,  2},
		{NULL, 	    NULL,            0}
	};

	YASLX_tablesetfunctions(S, functions);
	YASL_pop(S);

	return YASL_SUCCESS;
}