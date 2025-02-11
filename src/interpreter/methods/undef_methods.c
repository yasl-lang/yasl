#include "undef_methods.h"

#include "yasl.h"
#include "yasl_aux.h"

int undef_tobool(struct YASL_State *S) {
	YASLX_checknundef(S, "undef.tobool", 0);
	YASL_pushbool(S, false);
	return 1;
}

int undef_tostr(struct YASL_State *S) {
	YASLX_checknundef(S, "undef.tostr", 0);
	YASL_pushlit(S, "undef");
	return 1;
}
