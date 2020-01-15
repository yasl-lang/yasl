#include "yasl-std-math.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <data-structures/YASL_Set.h>
#include <interpreter/VM.h>

#include "prime/prime.h"
#include "interpreter/YASL_Object.h"
#include "yasl_conf.h"
#include "yasl.h"

const yasl_float YASL_PI = 3.14159265358979323851280895940618620443274267017841339111328125;
#if _MSC_VER
const yasl_float YASL_NAN = NAN;
const yasl_float YASL_INF = INFINITY;
#elseif __TINYC__
const yasl_float YASL_NAN = NAN;
const yasl_float YASL_INF = INF;
#elif defined(__TINYC__)
#define YASL_NAN nan("")
#define YASL_INF INFINITY
#else
const yasl_float YASL_NAN = 0.0 / 0.0;
const yasl_float YASL_INF = 1.0 / 0.0;
#endif

static bool YASL_top_isnumber(struct YASL_State *S) {
	return YASL_top_isinteger(S) || YASL_top_isfloat(S);
}

static int YASL_math_abs(struct YASL_State *S) {
	if (YASL_top_isinteger(S)) {
		yasl_int i = YASL_top_popinteger(S);
		if (i < 0) i = -i;
		return YASL_pushinteger(S, i);
	}

	if (YASL_top_isfloat(S)) {
		yasl_float f = YASL_top_popfloat(S);
		if (f < 0) f = -f;
		return YASL_pushfloat(S, f);
	}

	// TODO: error message
	return YASL_TYPE_ERROR;
}

static int YASL_math_exp(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, exp(n));
}

static int YASL_math_log(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, log(n));
}

static int YASL_math_sqrt(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, sqrt(n));
}

static int YASL_math_cos(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, cos(n));
}
static int YASL_math_sin(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, sin(n));
}
static int YASL_math_tan(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, tan(n));
}
static int YASL_math_acos(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, acos(n));
}
static int YASL_math_asin(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, asin(n));
}
static int YASL_math_atan(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, atan(n));
}

static int YASL_math_ceil(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, ceil(n));
}
static int YASL_math_floor(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}
	return YASL_pushfloat(S, floor(n));
}

static int YASL_math_max(struct YASL_State *S) {
	struct YASL_Object max = YASL_FLOAT(-YASL_INF);

	yasl_int num_va_args = vm_popint((struct VM *)S);
	if (num_va_args == 0) {
		YASL_pushfloat(S, -YASL_INF);
		return YASL_SUCCESS;
	}

	for (yasl_int i = 0; i < num_va_args; i++) {
		if (YASL_ISINT(vm_peek((struct VM *)S))) {
			yasl_int top = vm_popint((struct VM *)S);
			if ((top >= YASL_GETNUM(max))) {
				max = YASL_INT(top);
			}
		} else if (YASL_ISFLOAT(vm_peek((struct VM *)S))) {
			yasl_float top = vm_popfloat((struct VM *)S);
			if (top != top) {
				YASL_pushfloat(S, YASL_NAN);
				return YASL_SUCCESS;
			}
			if ((top >= YASL_GETNUM(max))) {
				max = YASL_FLOAT(top);
			}
		} else {
			YASL_PRINT_ERROR_BAD_ARG_TYPE("math.max", (int)i, Y_FLOAT, vm_peek((struct VM *)S).type);
			return YASL_TYPE_ERROR;
		}
	}
	vm_push((struct VM *)S, max);

	return YASL_SUCCESS;
}
static int YASL_math_min(struct YASL_State *S) {
	struct YASL_Object max = YASL_FLOAT(YASL_INF);

	yasl_int num_va_args = vm_popint((struct VM *)S);
	if (num_va_args == 0) {
		YASL_pushfloat(S, YASL_INF);
		return YASL_SUCCESS;
	}

	for (yasl_int i = 0; i < num_va_args; i++) {
		if (YASL_ISINT(vm_peek((struct VM *)S))) {
			yasl_int top = vm_popint((struct VM *)S);
			if ((top <= YASL_GETNUM(max))) {
				max = YASL_INT(top);
			}
		} else if (YASL_ISFLOAT(vm_peek((struct VM *)S))) {
			yasl_float top = vm_popfloat((struct VM *)S);
			if (top != top) {
				YASL_pushfloat(S, YASL_NAN);
				return YASL_SUCCESS;
			}
			if ((top <= YASL_GETNUM(max))) {
				max = YASL_FLOAT(top);
			}
		} else {
			YASL_PRINT_ERROR_BAD_ARG_TYPE("math.min", (int)i, Y_FLOAT, vm_peek((struct VM *)S).type);
			return YASL_TYPE_ERROR;
		}
	}
	vm_push((struct VM *)S, max);

	return YASL_SUCCESS;
}

static int YASL_math_deg(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}

	n *= (yasl_float)180.0/YASL_PI;
	return YASL_pushfloat(S, n);
}
static int YASL_math_rad(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_float n;
	if (YASL_top_isinteger(S)) {
		n = (yasl_float)YASL_top_popinteger(S);
	} else {
		n = YASL_top_popfloat(S);
	}

	n *= YASL_PI/(yasl_float)180.0;
	return YASL_pushfloat(S, n);
}

static int YASL_math_isprime(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_int n;
	if (YASL_top_isinteger(S)) {
		n = YASL_top_popinteger(S);
	} else {
		n = (yasl_int)YASL_top_popfloat(S);
	}

	int p = is_prime(n);
	if (p < 0) return YASL_pushundef(S);
	return YASL_pushboolean(S, p);
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
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_int a;
	if (YASL_top_isinteger(S)) {
		a = YASL_top_popinteger(S);
	} else {
		a = (yasl_int)YASL_top_popfloat(S);
	}

	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_int b;
	if (YASL_top_isinteger(S)) {
		b = YASL_top_popinteger(S);
	} else {
		b = (yasl_int)YASL_top_popfloat(S);
	}

	if (!(a > 0 && b > 0)) {
		// TODO: make this an error instead?
		return YASL_pushundef(S);
	}

	return YASL_pushinteger(S, gcd_helper(a, b));
}
static int YASL_math_lcm(struct YASL_State *S) {
	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_int a;
	if (YASL_top_isinteger(S)) {
		a = YASL_top_popinteger(S);
	} else {
		a = (yasl_int)YASL_top_popfloat(S);
	}

	if (!YASL_top_isnumber(S)) {
		return YASL_TYPE_ERROR;
	}

	yasl_int b;
	if (YASL_top_isinteger(S)) {
		b = YASL_top_popinteger(S);
	} else {
		b = (yasl_int)YASL_top_popfloat(S);
	}

	if (!(a > 0 && b > 0)) {
		return YASL_pushundef(S);
	}

	yasl_int gcd = gcd_helper(a, b);
	return YASL_pushinteger(S, b*a/gcd);
}

static int YASL_math_rand(struct YASL_State *S) {
	// rand() is only guarenteed to return a maximum of ~32000. Ensure all 64 bits are used
	yasl_int r = (yasl_int) rand() ^((yasl_int) rand() << 16) ^((yasl_int) rand() << 32) ^((yasl_int) rand() << 48);
	return YASL_pushinteger(S, r);
}

int YASL_load_math(struct YASL_State *S) {
	YASL_declglobal(S, "math");
	YASL_pushtable(S);
	YASL_setglobal(S, "math");

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "abs");
	YASL_pushcfunction(S, YASL_math_abs, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "exp");
	YASL_pushcfunction(S, YASL_math_exp, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "log");
	YASL_pushcfunction(S, YASL_math_log, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "pi");
	YASL_pushfloat(S, YASL_PI);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "nan");
	YASL_pushfloat(S, YASL_NAN);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "inf");
	YASL_pushfloat(S, YASL_INF);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "sqrt");
	YASL_pushcfunction(S, YASL_math_sqrt, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "cos");
	YASL_pushcfunction(S, YASL_math_cos, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "sin");
	YASL_pushcfunction(S, YASL_math_sin, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "tan");
	YASL_pushcfunction(S, YASL_math_tan, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "acos");
	YASL_pushcfunction(S, YASL_math_acos, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "asin");
	YASL_pushcfunction(S, YASL_math_asin, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "atan");
	YASL_pushcfunction(S, YASL_math_atan, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "ceil");
	YASL_pushcfunction(S, YASL_math_ceil, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "floor");
	YASL_pushcfunction(S, YASL_math_floor, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "max");
	YASL_pushcfunction(S, YASL_math_max, -1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "min");
	YASL_pushcfunction(S, YASL_math_min, -1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "deg");
	YASL_pushcfunction(S, YASL_math_deg, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "rad");
	YASL_pushcfunction(S, YASL_math_rad, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "isprime");
	YASL_pushcfunction(S, YASL_math_isprime, 1);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "gcd");
	YASL_pushcfunction(S, YASL_math_gcd, 2);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "lcm");
	YASL_pushcfunction(S, YASL_math_lcm, 2);
	YASL_settable(S);

	YASL_loadglobal(S, "math");
	YASL_pushlitszstring(S, "rand");
	YASL_pushcfunction(S, YASL_math_rand, 1);
	YASL_settable(S);

	return YASL_SUCCESS;
}

