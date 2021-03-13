#include "int_methods.h"

#include <stdio.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"

int int_toint(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.toint", 0);
	YASL_pushint(S, n);
	return 1;
}

int int_tobool(struct YASL_State *S) {
	YASL_UNUSED(YASLX_checknint(S, "int.tobool", 0));
	YASL_pushbool(S, true);
	return 1;
}

int int_tofloat(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.tofloat", 0);
	YASL_pushfloat(S, (yasl_float) n);
	return 1;
}

int int_tostr(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.tostr", 0);
	int len = snprintf(NULL, 0, "%" PRId64 "", n);
	char *ptr = (char *)malloc(len + 1);
	sprintf(ptr, "%" PRId64 "", n);
	YASL_pushlstr(S, ptr, len);
	free(ptr);
	return 1;
}
