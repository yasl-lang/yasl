#include "undef_methods.h"

#include "yasl.h"
#include "yasl_aux.h"

int undef_tostr(struct YASL_State *S) {
	YASLX_checkundef(S, "undef.tostr", 0);
	YASL_pushlitszstring(S, "undef");
	return YASL_SUCCESS;
}

int undef_tobool(struct YASL_State *S) {
	YASLX_checkundef(S, "undef.tobool", 0);
	YASL_pushbool(S, false);
	return YASL_SUCCESS;
}
