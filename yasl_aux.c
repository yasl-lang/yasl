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
	YASL_decllib_os(S);

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
		YASLX_print_err_bad_arg_type(S, name, n, "int", YASL_peekntypename(S, n));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknint(S, n);
}

yasl_float YASLX_checknfloat(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnfloat(S, n)) {
		YASLX_print_err_bad_arg_type(S, name, n, "float", YASL_peekntypename(S, n));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknfloat(S, n);
}

bool YASLX_checknbool(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnbool(S, n)) {
		YASLX_print_err_bad_arg_type(S, name, n, "bool", YASL_peekntypename(S, n));
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

static bool YASLX_functions_issentinal(struct YASLX_function functions) {
	return functions.name == NULL && functions.fn == NULL && functions.args == 0;
}

void YASLX_tablesetfunctions(struct YASL_State *S, const struct YASLX_function functions[]) {
	for (int i = 0; !YASLX_functions_issentinal(functions[i]); i++) {
		struct YASLX_function function = functions[i];
		YASL_pushlit(S, function.name);
		YASL_pushcfunction(S, function.fn, function.args);
		YASL_tableset(S);
	}
}