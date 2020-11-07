#include "undef_methods.h"

#include "yasl.h"
#include "yasl_aux.h"

void undef_tostr(struct YASL_State *S) {
	YASLX_checkundef(S, "undef.tostr", 0);
	YASL_pushlitszstring(S, "undef");
}

void undef_tobool(struct YASL_State *S) {
	YASLX_checkundef(S, "undef.tobool", 0);
	YASL_pushbool(S, false);
}
