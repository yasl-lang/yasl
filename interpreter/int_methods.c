#include "int_methods.h"

#include <stdio.h>

#include "yasl.h"
#include "yasl_aux.h"

void int_toint(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.toint", 0);
	YASL_pushint(S, n);
}

void int_tobool(struct YASL_State *S) {
	(void) YASLX_checknint(S, "int.tobool", 0);
	YASL_pushbool(S, true);
}

void int_tofloat(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "int.tofloat", 0);
	YASL_pushfloat(S, (yasl_float) n);
}

void int_tostr(struct YASL_State *S) {
	yasl_int n = YASLX_checkint(S, "int.tostr", 0);
	int len = snprintf(NULL, 0, "%" PRId64 "", n);
	char *ptr = (char *)malloc(len + 1);
	sprintf(ptr, "%" PRId64 "", n);
	YASL_pushlstr(S, ptr, len);
	free(ptr);
}
