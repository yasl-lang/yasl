#include "yasl-std-mt.h"

#include "yasl_aux.h"
#include "VM.h"

int YASL_mt_getmt(struct YASL_State *S) {
	vm_get_metatable((struct VM *)S);
	return 1;
}

int YASL_mt_setmt(struct YASL_State *S) {
	if (!YASL_istable(S)) {
		YASLX_print_err_bad_arg_type(S, "mt.set", 1, YASL_TABLE_NAME, YASL_peektypename(S));
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
		vm_print_err_type((struct VM *)S, "cannot set metatable for items of type %s.", YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return 1;
}

int YASL_decllib_mt(struct YASL_State *S) {
	YASL_declglobal(S, "mt");
	YASL_pushtable(S);
	YASL_setglobal(S, "mt");

	YASL_loadglobal(S, "mt");
	YASL_pushlit(S, "get");
	YASL_pushcfunction(S, YASL_mt_getmt, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "set");
	YASL_pushcfunction(S, YASL_mt_setmt, 2);
	YASL_tableset(S);
	YASL_pop(S);

	return YASL_SUCCESS;
}