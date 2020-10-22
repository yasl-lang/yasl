#include "int_methods.h"

#include <stdio.h>

#include "yasl.h"
#include "yasl_aux.h"

int int_toint(struct YASL_State *S) {
	yasl_int n = YASLX_checkint(S, "int.toint", 0);
	return YASL_pushint(S, n);
}

int int_tobool(struct YASL_State *S) {
	yasl_int n = YASLX_checkint(S, "int.tobool", 0);
	(void) n;
	return YASL_pushbool(S, true);
}

int int_tofloat(struct YASL_State *S) {
	yasl_int n = YASLX_checkint(S, "int.tofloat", 0);
	return YASL_pushfloat(S, (yasl_float) n);
}

int int_tostr(struct YASL_State *S) {
	yasl_int n = YASLX_checkint(S, "int.tostr", 0);
	int len = snprintf(NULL, 0, "%" PRId64 "", n);
	char *ptr = (char *)malloc(len + 1);
	sprintf(ptr, "%" PRId64 "", n);
	YASL_pushstring(S, ptr, len);
	return YASL_SUCCESS;
}
