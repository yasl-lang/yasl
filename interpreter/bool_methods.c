#include "bool_methods.h"

#include "yasl.h"
#include "yasl_aux.h"

void bool_tostr(struct YASL_State *S) {
	bool result = YASLX_checknbool(S, "bool.tostr", 0);
	const char *str = result ? "true" : "false";
	YASL_pushlit(S, str);
}

void bool_tobool(struct YASL_State *S) {
	(void)YASLX_checknbool(S, "bool.tobool", 0);
}
