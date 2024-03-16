#include "yasl_aux.h"
#include "yasl_include.h"

#include <string.h>

int YASLX_decllibs(struct YASL_State *S) {
	YASL_decllib_collections(S);
	YASL_decllib_error(S);
	YASL_decllib_io(S);
	YASL_decllib_math(S);
	YASL_decllib_mt(S);
	YASL_decllib_os(S);
	YASL_decllib_require(S);
	YASL_decllib_require_c(S);
	YASL_decllib_try(S);
	YASL_decllib_package(S);

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

void YASLX_print_err_bad_arg_type_n(struct YASL_State *S,
				const char *const fn_name,
				unsigned position,
				const char *const exp) {
	YASL_print_err(S, MSG_TYPE_ERROR "%s expected arg in position %d to be of type %s, got arg of type %s.", fn_name, position, exp, YASL_peekntypename(S, position));
}

void YASLX_throw_err_type(struct YASL_State *S) {
	YASL_throw_err(S, YASL_TYPE_ERROR);
}

void YASLX_throw_err_value(struct YASL_State *S) {
	YASL_throw_err(S, YASL_VALUE_ERROR);
}

void YASLX_print_and_throw_err_bad_arg_type_n(struct YASL_State *S,
				    const char *const fn_name,
				    unsigned position,
				    const char *const exp) {
	YASLX_print_err_bad_arg_type_n(S, fn_name, position, exp);
	YASLX_throw_err_type(S);
}

yasl_int YASLX_checknint(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnint(S, n)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, n, YASL_INT_NAME);
	}
	return YASL_peeknint(S, n);
}

yasl_float YASLX_checknfloat(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnfloat(S, n)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, n, YASL_FLOAT_NAME);
	}
	return YASL_peeknfloat(S, n);
}

bool YASLX_checknbool(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnbool(S, n)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, n, YASL_BOOL_NAME);
	}
	return YASL_peeknbool(S, n);
}

void YASLX_checknundef(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnundef(S, n)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, n, YASL_UNDEF_NAME);
	}
}

const char *YASLX_checknstr(struct YASL_State *S, const char *name, unsigned n, size_t *len) {
	if (!YASL_isnstr(S, n)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, n, YASL_STR_NAME);
	}
	return YASL_peeknstr(S, n, len);
}

void *YASLX_checknuserdata(struct YASL_State *S, const char *tag, const char *name, unsigned n) {
	if (!YASL_isnuserdata(S, tag, n)) {
		YASLX_print_and_throw_err_bad_arg_type_n(S, name, n, tag);
	}
	return YASL_peeknuserdata(S, n);
}

yasl_int YASLX_checknoptint(struct YASL_State *S, const char *fn_name, unsigned n, yasl_int default_val) {
	if (YASL_isnundef(S, n)) {
		return default_val;
	}

	return YASLX_checknint(S, fn_name, n);
}

yasl_float YASLX_checknoptfloat(struct YASL_State *S, const char *fn_name, unsigned n, yasl_float default_val) {
	if (YASL_isnundef(S, n)) {
		return default_val;
	}

	return YASLX_checknfloat(S, fn_name, n);
}

bool YASLX_checknoptbool(struct YASL_State *S, const char *fn_name, unsigned n, bool default_val) {
	if (YASL_isnundef(S, n)) {
		return default_val;
	}

	return YASLX_checknbool(S, fn_name, n);
}

const char *YASLX_checknoptstrz(struct YASL_State *S, const char *fn_name, unsigned n, size_t *len, const char *default_val) {
	if (YASL_isnundef(S, n)) {
		*len = strlen(default_val);
		return default_val;
	}

	return YASLX_checknstr(S, fn_name, n, len);
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