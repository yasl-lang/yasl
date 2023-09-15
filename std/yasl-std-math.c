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

#define YASL_NUM_NAME YASL_FLOAT_NAME " or " YASL_INT_NAME

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

static bool YASL_isnnum(struct YASL_State *S, unsigned n) {
	return YASL_isnint(S, n) || YASL_isnfloat(S, n);
}

static yasl_float YASL_peeknnum(struct YASL_State *S, unsigned n) {
	if (YASL_isnint(S, n)) return (yasl_float)YASL_peeknint(S, n);
	else return YASL_peeknfloat(S, n);
}

static yasl_float YASLX_checknnum(struct YASL_State *S, const char *name, unsigned n) {
	if (!YASL_isnnum(S, n)) {
		YASLX_print_err_bad_arg_type(S, name, n, "float", YASL_peekntypename(S, n));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return YASL_peeknnum(S, n);
}

static int YASL_math_abs(struct YASL_State *S) {
	if (YASL_isnint(S, 0)) {
		yasl_int i = YASL_popint(S);
		if (i < 0) i = -i;
		YASL_pushint(S, i);
		return 1;
	}

	if (YASL_isnfloat(S, 0)) {
		yasl_float f = YASL_popfloat(S);
		if (f < 0) f = -f;
		YASL_pushfloat(S, f);
		return 1;
	}

	vm_print_err_bad_arg_type_name((struct VM*)S,"math.abs", 0, YASL_NUM_NAME, YASL_peektypename(S));
	YASL_throw_err(S, YASL_TYPE_ERROR);
}

#define DEFINE_MATH_UN_FLOAT_FUN(name) static int YASL_math_##name(struct YASL_State *S) {\
	yasl_float n = YASLX_checknnum(S, "math." #name, 0);\
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

	yasl_int num_va_args = YASL_peekvargscount(S);
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
			vm_print_err_bad_arg_type_name((struct VM*)S, "math.max", (int)(num_va_args - i - 1), YASL_NUM_NAME, YASL_peektypename(S));
			YASL_throw_err(S, YASL_TYPE_ERROR);
		}
	}
	vm_push((struct VM *)S, max);
	return 1;
}
static int YASL_math_min(struct YASL_State *S) {
	struct YASL_Object max = YASL_FLOAT(YASL_INF);

	yasl_int num_va_args = YASL_peekvargscount(S);
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
			vm_print_err_bad_arg_type_name((struct VM*)S,"math.min", (int)(num_va_args - i - 1), YASL_NUM_NAME, YASL_peektypename(S));
			YASL_throw_err(S, YASL_TYPE_ERROR);
		}
	}
	vm_push((struct VM *)S, max);
	return 1;
}

static int YASL_math_deg(struct YASL_State *S) {
	yasl_float n = YASLX_checknnum(S, "math.deg", 0);

	n *= (yasl_float)180.0/YASL_PI;
	YASL_pushfloat(S, n);
	return 1;
}
static int YASL_math_rad(struct YASL_State *S) {
	yasl_float n = YASLX_checknnum(S, "math.rad", 0);

	n *= YASL_PI/(yasl_float)180.0;
	YASL_pushfloat(S, n);
	return 1;
}

static int YASL_math_isprime(struct YASL_State *S) {
	if (!YASL_isnnum(S, 0)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.isprime", 0, YASL_NUM_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int n;
	if (YASL_isnint(S, 0)) {
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
	if (!YASL_isnnum(S, 1)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.gcd", 1, YASL_NUM_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int a;
	if (YASL_isnint(S, 1)) {
		a = YASL_popint(S);
	} else {
		a = (yasl_int)YASL_popfloat(S);
	}

	if (!YASL_isnnum(S, 0)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.gcd", 0, YASL_NUM_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int b;
	if (YASL_isnint(S, 0)) {
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
	if (!YASL_isnnum(S, 1)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.lcm", 1, YASL_FLOAT_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int a;
	if (YASL_isnint(S, 1)) {
		a = YASL_popint(S);
	} else {
		a = (yasl_int)YASL_popfloat(S);
	}

	if (!YASL_isnnum(S, 0)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.lcm", 0, YASL_FLOAT_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_int b;
	if (YASL_isnint(S, 0)) {
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

static int YASL_math_clamp(struct YASL_State *S) {
	if (!YASL_isnnum(S, 2)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.clamp", 2, YASL_NUM_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_float h;
	bool is_h_int; 
	if (YASL_isnint(S, 2)) {
		h = (yasl_float)YASL_popint(S);
		is_h_int = true;
	} else {
		h = YASL_popfloat(S);
		is_h_int = false;
	}

	if (!YASL_isnnum(S, 1)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.clamp", 1, YASL_NUM_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_float l;
	bool is_l_int;
	if (YASL_isnint(S, 1)) {
		l = (yasl_float)YASL_popint(S);
		is_l_int = true;
	} else {
		l = YASL_popfloat(S);
		is_l_int = false;
	}

	if (!YASL_isnnum(S, 0)) {
		vm_print_err_bad_arg_type_name((struct VM*)S,"math.clamp", 0, YASL_NUM_NAME, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	yasl_float v;
	bool is_v_int;
	if (YASL_isnint(S, 0)) {
		v = (yasl_float)YASL_popint(S);
		is_v_int = true;
	} else {
		v = YASL_popfloat(S);
		is_v_int = false;
	}

	if(v < l) {
		is_l_int ? YASL_pushint(S, (yasl_int)l) : YASL_pushfloat(S, l);
	} else if (v > h) {
		is_h_int ? YASL_pushint(S, (yasl_int)h) : YASL_pushfloat(S, h);
	} else {
		is_v_int ? YASL_pushint(S, (yasl_int)v) : YASL_pushfloat(S, v);
	}
	// the statement: is_*_int ? YASL_pushint(S, (yasl_int)*) : YASL_pushfloat(S, *);
	// Checks if the above variables v, l, h are ints, 
	// so as to avoid pushing something like 10.0 instead of 10

	return 1;
}

static int YASL_math_rand(struct YASL_State *S) {
	// rand() is only guarenteed to return a maximum of ~32000. Ensure all 64 bits are used
	yasl_int r = (yasl_int) rand() ^((yasl_int) rand() << 16) ^((yasl_int) rand() << 32) ^((yasl_int) rand() << 48);
	YASL_pushint(S, r);
	return 1;
}

static int YASL_math_seed(struct YASL_State *S) {
	yasl_int n = YASLX_checknint(S, "math.seed", 0);
	srand(n);
	return 0;
}

void YASL_decllib_math(struct YASL_State *S) {
	YASL_declglobal(S, "math");
	YASL_pushtable(S);
	YASL_setglobal(S, "math");

	YASL_loadglobal(S, "math");

	YASL_pushlit(S, "pi");
	YASL_pushfloat(S, YASL_PI);
	YASL_tableset(S);

	YASL_pushlit(S, "nan");
	YASL_pushfloat(S, YASL_NAN);
	YASL_tableset(S);

	YASL_pushlit(S, "inf");
	YASL_pushfloat(S, YASL_INF);
	YASL_tableset(S);

	struct YASLX_function functions[] = {
		{"abs",     YASL_math_abs,     1},
		{"exp",     YASL_math_exp,     1},
		{"log",     YASL_math_log,     1},
		{"sqrt",    YASL_math_sqrt,    1},
		{"cos",     YASL_math_cos,     1},
		{"sin",     YASL_math_sin,     1},
		{"tan",     YASL_math_tan,     1},
		{"acos",    YASL_math_acos,    1},
		{"asin",    YASL_math_asin,    1},
		{"atan",    YASL_math_atan,    1},
		{"ceil",    YASL_math_ceil,    1},
		{"floor",   YASL_math_floor,   1},
		{"max",     YASL_math_max,     -1},
		{"min",     YASL_math_min,     -1},
		{"deg",     YASL_math_deg,     1},
		{"rad",     YASL_math_rad,     1},
		{"isprime", YASL_math_isprime, 1},
		{"gcd",     YASL_math_gcd,     2},
		{"lcm",     YASL_math_lcm,     2},
		{"clamp",   YASL_math_clamp,   3},
		{"rand",    YASL_math_rand,    0},
		{"seed",    YASL_math_seed,    1},
		{NULL, 	    NULL,              0}
	};

	YASLX_tablesetfunctions(S, functions);
	YASL_pop(S);
}
