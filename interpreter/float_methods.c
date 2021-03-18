#include "float_methods.h"

#include <string.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_float.h"

int float_toint(struct YASL_State *S) {
	yasl_float val = YASLX_checknfloat(S, "float.toint", 0);
	YASL_pushint(S, (yasl_int)val);
	return 1;
}

int float_tobool(struct YASL_State *S) {
	yasl_float val = YASLX_checknfloat(S, "float.tobool", 0);
	YASL_pushbool(S, val == val);
	return 1;
}


int float_tofloat(struct YASL_State *S) {
	yasl_float val = YASLX_checknfloat(S, "float.tofloat", 0);
	YASL_pushfloat(S, val);
	return 1;
}

int float_tostr(struct YASL_State *S) {
	yasl_float val = YASLX_checknfloat(S, "float.tostr", 0);
	const char *ptr = float64_to_str(val);
	YASL_pushzstr(S, ptr);
	free((void *)ptr);
	return 1;
}
