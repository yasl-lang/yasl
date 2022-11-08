#include "bool_methods.h"

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_include.h"

int bool_tobool(struct YASL_State *S) {
	YASL_UNUSED(YASLX_checknbool(S, "bool.tobool", 0));
	return 1;
}

int bool_tostr(struct YASL_State *S) {
	bool result = YASLX_checknbool(S, "bool.tostr", 0);
	const char *str = result ? "true" : "false";
	YASL_pushlit(S, str);
	return 1;
}

int bool___bor(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bor", 0);
	bool right = YASLX_checknbool(S, "bool.__bor", 1);

	YASL_pushbool(S, left || right);
	return 1;
}

int bool___band(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bor", 0);
	bool right = YASLX_checknbool(S, "bool.__bor", 1);

	YASL_pushbool(S, left && right);
	return 1;
}

int bool___bandnot(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bor", 0);
	bool right = YASLX_checknbool(S, "bool.__bor", 1);

	YASL_pushbool(S, left && !right);
	return 1;
}

int bool___bxor(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bor", 0);
	bool right = YASLX_checknbool(S, "bool.__bor", 1);

	YASL_pushbool(S, left ^ right);
	return 1;
}

int bool___bnot(struct YASL_State *S) {
	bool left = YASLX_checknbool(S, "bool.__bor", 0);

	YASL_pushbool(S, !left);
	return 1;
}
