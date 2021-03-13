#include "yasl-std-math.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <data-structures/YASL_Set.h>
#include <interpreter/VM.h>

#include "util/prime.h"
#include "interpreter/YASL_Object.h"
#include "yasl_conf.h"
#include "yasl.h"
#include "yasl_aux.h"

const yasl_float YASL_PI = 3.14159265358979323851280895940618620443274267017841339111328125;
#if _MSC_VER
const yasl_float YASL_NAN = NAN;
const yasl_float YASL_INF = INFINITY;
#elif defined(__TINYC__)
#define YASL_NAN nan("")
#define YASL_INF INFINITY
#else
const yasl_float YASL_NAN = 0.0 / 0.0;
const yasl_float YASL_INF = 1.0 / 0.0;
#endif

static bool YASL_isnum(struct YASL_State *S) {
	return YASL_isint(S) || YASL_isfloat(S);
}

static yasl_float YASL_popnum(struct YASL_State *S) {
	if (YASL_isint(S)) return (yasl_float)YASL_popint(S);
	else return YASL_popfloat(S);
}

/*
static yasl_float YASL_peeknum(struct YASL_State *S) {
	if (YASL_isint(S)) return (yasl_float)YASL_peekint(S);
	else return YASL_peekfloat(S);
}
*/

static yasl_float YASLX_checknum(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isnum(S)) {
		YASLX_print_err_bad_arg_type(S, name, pos, "float", YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_popnum(S);
}

static int YASL_math_abs(struct YASL_State *S) {
	if (YASL_isint(S)) {
		yasl_int i = YASL_popint(S);
		if (i < 0) i = -i;
		YASL_pushint(S, i);
		return 1;
	}

	if (YASL_isfloat(S)) {
		yasl_float f = YASL_popfloat(S);
		if (f < 0) f = -f;
		YASL_pushfloat(S, f);
		return 1;
	}

	vm_print_err_bad_arg_type_name((struct VM*)S,"math.abs", 0, YASL_FLOAT_NAME, YASL_peektypename(S));
	YASL_throw_err(S, YASL_TYPE_ERROR);
}

#define DEFINE_MATH_UN_FLOAT_FUN(name) static int YASL_math_##name(struct YASL_State *S) {\
	yasl_float n = YASLX_checknum(S, "math." #name, 0);\
	YASL_pushfloat(S, name(n));\
	return 1;\
}

DEFINE_MATH_UN_FLOAT_FUN(exp);
DEFINE_MATH_UN_FLOAT_FUN(log);
DEFINE_MATH_UN_FLOAT_FUN(sqrt);
DEFINE_MATH_UN_FLOAT_FUN(sin);
DEFINE_MATH_UN_FLOAT_FUN(cos);
DEFINE_MATH_UN_FLOAT_FUN(tan);
DEFINE_MATH_UN_FLOAT_FUN(asin);
DEFINE_MATH_UN_FLOAT_FUN(acos);
DEFINE_MATH_UN_FLOAT_FUN(atan);
DEFINE_MATH_UN_FLOAT_FUN(ceil);
DEFINE_MATH_UN_FLOAT_FUN(floor);

static int YASL_math_max(struct YASL_State *S) {
	struct YASL_Object max = YASL_FLOAT(-YASL_INF);

	yasl_int num_va_args = YASL_popint(S);
	if (num_va_args == 0) {
		YASL_pushfloat(S, -YASL_INF);
		return 1;
	}

	for (yasl_int i = 0; i < num_va_args; i++) {
		if (vm_isint((struct VM *)S)) {
			yasl_int top = vm_popint((struct VM *)S);
			if ((top >= obj_getnum(&max))) {
				max = YASL_INT(top);
			}
		} else if (vm_isfloat((struct VM *)S)) {
			yasl_float top = vm_popfloat((struct VM *)S);
			if (top != top) {
				YASL_pushfloat(S, YASL_NAN);
				return 1;
			}
			if ((top >= obj_getnum(&max))) {
				max = YASL_FLOAT(top);
			}
		} else {
			vm_print_err_bad_arg_type_name((struct VM*)S, "math.max", (int)(num_va_args - i - 1), YASL_FLOAT_NAME, YASL_peektypename(S));
			YASL_throw_err(S, YASL_TYPE_ERROR);
		}
	}
	vm_push((struct VM *)S, max);
	return 1;
}
static int YASL_math_min(struct YASL_State *S) {
	struct YASL_Object max = YASL_FLOAT(YASL_INF);

	yasl_int num_va_args = YASL_popint(S);
	if (num_va_args == 0) {
		YASL_pushfloat(S, YASL_INF);
		return 1;
	}

	for (yasl_int i = 0; i < num_va_args; i++) {
		if (vm_isint((struct VM *)S)) {
			yasl_int top = vm_popint((struct VM *)S);
			if ((top <= obj_getnum(&max))) {
				max = YASL_INT(top);
			}
		} else if (vm_isfloat((struct VM *)S)) {
			yasl_float top = vm_popfloat((struct VM *)S);
			if (top != top) {
				YASL_pushfloat(S, YASL_NAN);
				return 1;
			}
			if ((top <= obj_getnum(&max))) {
				max = YASL_FLOAT(top);
			}
		} else {
			vm_print_err_bad_arg_type_name((struct VM*)S,"math.min", (int)(num_va_args - i - 1), YASL_FLOAT_NAME, YASL_peektypename(S));
			YASL_throw_err(S, YASL_TYPE_ERROR);
		}
	}
	vm_push((struct VM *)S, max);
	return 1;
}

static int YASL_math_deg(struct YASL_State *S) {
	yasl_float n = YASLX_checknum(S, "math.deg", 0);

	n *= (yasl_float)180.0/YASL_PI;
	YASL_pushfloat(S, n);
	return 1;
}
static int YASL_math_rad(struct YASL_State *S) {
	yasl_float n = YASLX_checknum(S, "math.rad", 0);

	n *= YASL_PI/(yasl_float)180.0;
	YASL_pushfloat(S, n);
	return 1;
}

static int YASL_math_isprime(struct YASL_State *S) {
	if (!YASL_isnum(S)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.isprime", 0, YASL_FLOAT_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int n;
	if (YASL_isint(S)) {
		n = YASL_popint(S);
	} else {
		n = (yasl_int)YASL_popfloat(S);
	}

	int p = is_prime(n);
	if (p < 0) YASL_pushundef(S);
	else YASL_pushbool(S, (bool)p);
	return 1;
}
yasl_int gcd_helper(yasl_int a, yasl_int b) {
	if (a < b) {
		yasl_int tmp = a;
		a = b;
		b = tmp;
	}

	yasl_int r = 1;
	while(r != 0) {
		r = a % b;
		a = b;
		b = r;
	};
	return a;
}
static int YASL_math_gcd(struct YASL_State *S) {
	if (!YASL_isnum(S)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.gcd", 1, YASL_FLOAT_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int a;
	if (YASL_isint(S)) {
		a = YASL_popint(S);
	} else {
		a = (yasl_int)YASL_popfloat(S);
	}

	if (!YASL_isnum(S)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.gcd", 0, YASL_FLOAT_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int b;
	if (YASL_isint(S)) {
		b = YASL_popint(S);
	} else {
		b = (yasl_int)YASL_popfloat(S);
	}

	if (!(a > 0 && b > 0)) {
		// TODO: make this an error instead?
		YASL_pushundef(S);
	} else {
		YASL_pushint(S, gcd_helper(a, b));
	}
	return 1;
}
static int YASL_math_lcm(struct YASL_State *S) {
	if (!YASL_isnum(S)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.lcm", 1, YASL_FLOAT_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int a;
	if (YASL_isint(S)) {
		a = YASL_popint(S);
	} else {
		a = (yasl_int)YASL_popfloat(S);
	}

	if (!YASL_isnum(S)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.lcm", 0, YASL_FLOAT_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int b;
	if (YASL_isint(S)) {
		b = YASL_popint(S);
	} else {
		b = (yasl_int)YASL_popfloat(S);
	}

	if (!(a > 0 && b > 0)) {
		YASL_pushundef(S);
	} else {
		yasl_int gcd = gcd_helper(a, b);
		YASL_pushint(S, b*a/gcd);
	}
	return 1;
}

static int YASL_math_rand(struct YASL_State *S) {
	// rand() is only guarenteed to return a maximum of ~32000. Ensure all 64 bits are used
	yasl_int r = (yasl_int) rand() ^((yasl_int) rand() << 16) ^((yasl_int) rand() << 32) ^((yasl_int) rand() << 48);
	YASL_pushint(S, r);
	return 1;
}

int YASL_decllib_math(struct YASL_State *S) {
	YASL_declglobal(S, "math");
	YASL_pushtable(S);
	YASL_setglobal(S, "math");

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "abs");
	YASL_pushcfunction(S, YASL_math_abs, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "exp");
	YASL_pushcfunction(S, YASL_math_exp, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "log");
	YASL_pushcfunction(S, YASL_math_log, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "pi");
	YASL_pushfloat(S, YASL_PI);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "nan");
	YASL_pushfloat(S, YASL_NAN);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "inf");
	YASL_pushfloat(S, YASL_INF);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "sqrt");
	YASL_pushcfunction(S, YASL_math_sqrt, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "cos");
	YASL_pushcfunction(S, YASL_math_cos, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "sin");
	YASL_pushcfunction(S, YASL_math_sin, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "tan");
	YASL_pushcfunction(S, YASL_math_tan, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "acos");
	YASL_pushcfunction(S, YASL_math_acos, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "asin");
	YASL_pushcfunction(S, YASL_math_asin, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "atan");
	YASL_pushcfunction(S, YASL_math_atan, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "ceil");
	YASL_pushcfunction(S, YASL_math_ceil, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "floor");
	YASL_pushcfunction(S, YASL_math_floor, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "max");
	YASL_pushcfunction(S, YASL_math_max, -1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "min");
	YASL_pushcfunction(S, YASL_math_min, -1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "deg");
	YASL_pushcfunction(S, YASL_math_deg, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "rad");
	YASL_pushcfunction(S, YASL_math_rad, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "isprime");
	YASL_pushcfunction(S, YASL_math_isprime, 1);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "gcd");
	YASL_pushcfunction(S, YASL_math_gcd, 2);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "lcm");
	YASL_pushcfunction(S, YASL_math_lcm, 2);
	YASL_tableset(S);

	YASL_loadglobal(S, "math");
	YASL_pushlit(S, "rand");
	YASL_pushcfunction(S, YASL_math_rand, 1);
	YASL_tableset(S);

	return YASL_SUCCESS;
}

