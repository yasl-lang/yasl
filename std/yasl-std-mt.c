#include "yasl-std-mt.h"

#include "VM.h"

void YASL_mt_getmt(struct YASL_State *S) {
	vm_get_metatable((struct VM *)S);
}

void YASL_mt_setmt(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		vm_print_err_bad_arg_type((struct VM*)S,"mt.setmt", 1, Y_TABLE, vm_peek((struct VM *)S).type);
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	struct YASL_Object mt = vm_pop((struct VM *)S);
	switch (YASL_peektype(S)) {
	case Y_USERDATA:
	case Y_USERDATA_W:
	case Y_LIST:
	case Y_LIST_W:
	case Y_TABLE:
	case Y_TABLE_W:
		ud_setmt(YASL_GETUSERDATA(vm_peek((struct VM *)S)), YASL_GETUSERDATA(mt));
		break;
	default:
		vm_print_err_type((struct VM *)S, "cannot set metatable for value of type %s.", YASL_TYPE_NAMES[YASL_peektype(S)]);
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
}

int YASL_decllib_mt(struct YASL_State *S) {
	YASL_declglobal(S, "mt");
	YASL_pushtable(S);
	YASL_setglobal(S, "mt");

	YASL_loadglobal(S, "mt");
	YASL_pushlitszstring(S, "get");
	YASL_pushcfunction(S, YASL_mt_getmt, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "mt");
	YASL_pushlitszstring(S, "set");
	YASL_pushcfunction(S, YASL_mt_setmt, 2);
	YASL_tableset(S);

	return YASL_SUCCESS;
}