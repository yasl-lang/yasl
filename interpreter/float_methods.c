#include "float_methods.h"

#include <string.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_float.h"

int float_toint(struct YASL_State *S) {
	yasl_float val = YASLX_checkfloat(S, "float.toint", 0);
	return YASL_pushint(S, (yasl_int)val);
}

int float_tobool(struct YASL_State *S) {
	yasl_float val = YASLX_checkfloat(S, "float.tobool", 0);
	return YASL_pushbool(S, val == val);
}


int float_tofloat(struct YASL_State *S) {
	yasl_float val = YASLX_checkfloat(S, "float.tofloat", 0);
	return YASL_pushfloat(S, val);
}

int float_tostr(struct YASL_State *S) {
	yasl_float val = YASLX_checkfloat(S, "float.tostr", 0);
	const char *ptr = float64_to_str(val);
	return YASL_pushstring(S, ptr, strlen(ptr));
}
