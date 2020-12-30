#include "yasl_aux.h"
#include "yasl_include.h"

int YASLX_decllibs(struct YASL_State *S) {
	YASL_decllib_collections(S);
	YASL_decllib_io(S);
	YASL_decllib_math(S);
	YASL_decllib_require(S);
	YASL_decllib_require_c(S);
	YASL_decllib_mt(S);

	return YASL_SUCCESS;
}

void YASLX_print_err_bad_arg_type(struct YASL_State *S,
				 const char *const fn_name,
				 int position,
				 const char *const exp,
				 const char *const act) {
	YASL_print_err(S, MSG_TYPE_ERROR "%s expected arg in position %d to be of type %s, got arg of type %s.", fn_name, position, exp, act);
}

yasl_int YASLX_checkint(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isint(S)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "int", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_popint(S);
}

yasl_int YASLX_checknint(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnint(S, pos)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "int", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknint(S, pos);
}

yasl_float YASLX_checkfloat(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isfloat(S)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "float", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_popfloat(S);
}

yasl_float YASLX_checknfloat(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnfloat(S, pos)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "float", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknfloat(S, pos);
}

bool YASLX_checkbool(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isbool(S)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "bool", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_popbool(S);
}

bool YASLX_checknbool(struct YASL_State *S, const char *name, unsigned pos) {
	if (!YASL_isnbool(S, pos)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "bool", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknbool(S, pos);
}

void YASLX_checkundef(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isundef(S)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "undef", YASL_peektypestr(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	YASL_pop(S);
}
