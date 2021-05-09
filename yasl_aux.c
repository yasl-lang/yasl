#include "yasl_aux.h"
#include "yasl_include.h"

int YASLX_decllibs(struct YASL_State *S) {
	YASL_decllib_collections(S);
	YASL_decllib_error(S);
	YASL_decllib_io(S);
	YASL_decllib_math(S);
	YASL_decllib_require(S);
	YASL_decllib_require_c(S);
	YASL_decllib_mt(S);

	return YASL_SUCCESS;
}

void YASLX_initglobal(struct YASL_State *S, const char *name) {
	YASL_declglobal(S, name);
	YASL_setglobal(S, name);
}

void YASLX_print_err_bad_arg_type(struct YASL_State *S,
				 const char *const fn_name,
				 int position,
				 const char *const exp,
				 const char *const act) {
	YASL_print_err(S, MSG_TYPE_ERROR "%s expected arg in position %d to be of type %s, got arg of type %s.", fn_name, position, exp, act);
}

yasl_int YASLX_checknint(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnint(S, n)) {
		YASLX_print_err_bad_arg_type(S, name, n, "int", YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknint(S, n);
}

yasl_float YASLX_checknfloat(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnfloat(S, n)) {
		YASLX_print_err_bad_arg_type(S, name, n, "float", YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknfloat(S, n);
}

bool YASLX_checknbool(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnbool(S, n)) {
		YASLX_print_err_bad_arg_type(S, name, n, "bool", YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknbool(S, n);
}

void YASLX_checknundef(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnundef(S, n)) {
		YASLX_print_err_bad_arg_type(S, name, n, "undef", YASL_peekntypename(S, n));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
}

void *YASLX_checknuserdata(struct YASL_State *S, const char *tag, const char *name, unsigned n) {
	if (!YASL_isnuserdata(S, tag, n)) {
		YASLX_print_err_bad_arg_type(S, name, n, tag, YASL_peekntypename(S, n));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknuserdata(S, n);
}
