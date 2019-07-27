#include "yasl-std-math.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "prime/prime.h"
#include "interpreter/YASL_Object.h"
#include "yasl_conf.h"
#include "yasl.h"

#define POP_NUMBER(state, obj_name, fn_name) \
					struct YASL_Object *obj_name = YASL_popobject(state); \
                    if (!YASL_ISNUM(*obj_name)) { \
                        printf("%s(...) expected first argument of numerical type, got %s.\n", \
                                fn_name, YASL_TYPE_NAMES[obj_name->type] ); \
						return -1; \
                    }

const yasl_float YASL_PI = 3.14159265358979323851280895940618620443274267017841339111328125;
#if _MSC_VER
const yasl_float YASL_NAN = NAN;
const yasl_float YASL_INF = INFINITY;
#else
const yasl_float YASL_NAN = 0.0 / 0.0;
const yasl_float YASL_INF = 1.0 / 0.0;
#endif

static int YASL_math_abs(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.abs");

	if (YASL_ISINT(*num)) {
		yasl_int i = YASL_GETINT(*num);
		if (i < 0) i = -i;
		return YASL_pushinteger(S, i);
	} else {
		yasl_float n = YASL_GETFLOAT(*num);
		if (n < 0) n = -n;
		return YASL_pushfloat(S, n);
	}
}

static int YASL_math_exp(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.exp");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, exp(n));
}

static int YASL_math_log(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.log");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, log(n));
}

static int YASL_math_sqrt(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.sqrt");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, sqrt(n));
}

static int YASL_math_cos(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.cos");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, cos(n));
}
static int YASL_math_sin(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.sin");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, sin(n));
}
static int YASL_math_tan(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.tan");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, tan(n));
}
static int YASL_math_acos(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.acos");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, acos(n));
}
static int YASL_math_asin(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.asin");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, asin(n));
}
static int YASL_math_atan(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.atan");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, atan(n));
}

static int YASL_math_ceil(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.ceil");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, ceil(n));
}
static int YASL_math_floor(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.floor");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}
	return YASL_pushfloat(S, floor(n));
}

static int YASL_math_deg(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.deg");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}

	n *= (yasl_float)180.0/YASL_PI;
	return YASL_pushfloat(S, n);
}
static int YASL_math_rad(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.rad");

	yasl_float n;
	if (YASL_ISINT(*num)) {
		n = (yasl_float)YASL_GETINT(*num);;
	} else {
		n = YASL_GETFLOAT(*num);
	}

	n *= YASL_PI/(yasl_float)180.0;
	return YASL_pushfloat(S, n);
}

static int YASL_math_isprime(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.isprime");

	yasl_int n;
	if (YASL_ISFLOAT(*num)) {
		n = num->value.dval;
	} else {
		n = num->value.ival;
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
	POP_NUMBER(S, numA, "math.gcd");
	POP_NUMBER(S, numB, "math.gcd");

	yasl_int a = 0, b = 0;
	if (YASL_ISFLOAT(*numA)) {
		a = numA->value.dval;
	} else {
		a = numA->value.ival;
	}
	if (YASL_ISFLOAT(*numB)) {
		b = numB->value.dval;
	} else {
		b = numB->value.ival;
	}
	if (!(a > 0 && b > 0)) {
		return YASL_pushundef(S);
	}

	return YASL_pushinteger(S, gcd_helper(a, b));
}
static int YASL_math_lcm(struct YASL_State *S) {
	POP_NUMBER(S, numA, "math.lcm");
	POP_NUMBER(S, numB, "math.lcm");

	yasl_int a = 0, b = 0;
	if (YASL_ISFLOAT(*numA)) {
		a = numA->value.dval;
	} else {
		a = numA->value.ival;
	}
	if (YASL_ISFLOAT(*numB)) {
		b = numB->value.dval;
	} else {
		b = numB->value.ival;
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
	struct YASL_Object *math = YASL_Table();

	struct YASL_Object *abs_str = YASL_LiteralString("abs");
	struct YASL_Object *abs_fn = YASL_CFunction(YASL_math_abs, 1);
	YASL_Table_set(math, abs_str, abs_fn);

	struct YASL_Object *exp_str = YASL_LiteralString("exp");
	struct YASL_Object *exp_fn = YASL_CFunction(YASL_math_exp, 1);
	YASL_Table_set(math, exp_str, exp_fn);

	struct YASL_Object *log_str = YASL_LiteralString("log");
	struct YASL_Object *log_fn = YASL_CFunction(YASL_math_log, 1);
	YASL_Table_set(math, log_str, log_fn);

	struct YASL_Object *pi_str = YASL_LiteralString("pi");
	struct YASL_Object *pi_val = (struct YASL_Object *)malloc(sizeof(struct YASL_Object));
	pi_val->type = Y_FLOAT;
	pi_val->value.dval = YASL_PI;
	YASL_Table_set(math, pi_str, pi_val);

	struct YASL_Object *nan_str = YASL_LiteralString("nan");
	struct YASL_Object *nan_val = (struct YASL_Object *)malloc(sizeof(struct YASL_Object));
	nan_val->type = Y_FLOAT;
	nan_val->value.dval = YASL_NAN;
	YASL_Table_set(math, nan_str, nan_val);

	struct YASL_Object *inf_str = YASL_LiteralString("inf");
	struct YASL_Object *inf_val = (struct YASL_Object *)malloc(sizeof(struct YASL_Object));
	inf_val->type = Y_FLOAT;
	inf_val->value.dval = YASL_INF;
	YASL_Table_set(math, inf_str, inf_val);

	struct YASL_Object *sqrt_str = YASL_LiteralString("sqrt");
	struct YASL_Object *sqrt_fn = YASL_CFunction(YASL_math_sqrt, 1);
	YASL_Table_set(math, sqrt_str, sqrt_fn);

	struct YASL_Object *cos_str = YASL_LiteralString("cos");
	struct YASL_Object *cos_fn = YASL_CFunction(YASL_math_cos, 1);
	YASL_Table_set(math, cos_str, cos_fn);
	struct YASL_Object *sin_str = YASL_LiteralString("sin");
	struct YASL_Object *sin_fn = YASL_CFunction(YASL_math_sin, 1);
	YASL_Table_set(math, sin_str, sin_fn);
	struct YASL_Object *tan_str = YASL_LiteralString("tan");
	struct YASL_Object *tan_fn = YASL_CFunction(YASL_math_tan, 1);
	YASL_Table_set(math, tan_str, tan_fn);
	struct YASL_Object *acos_str = YASL_LiteralString("acos");
	struct YASL_Object *acos_fn = YASL_CFunction(YASL_math_acos, 1);
	YASL_Table_set(math, acos_str, acos_fn);
	struct YASL_Object *asin_str = YASL_LiteralString("asin");
	struct YASL_Object *asin_fn = YASL_CFunction(YASL_math_asin, 1);
	YASL_Table_set(math, asin_str, asin_fn);
	struct YASL_Object *atan_str = YASL_LiteralString("atan");
	struct YASL_Object *atan_fn = YASL_CFunction(YASL_math_atan, 1);
	YASL_Table_set(math, atan_str, atan_fn);

	struct YASL_Object *ceil_str = YASL_LiteralString("ceil");
	struct YASL_Object *ceil_fn = YASL_CFunction(YASL_math_ceil, 1);
	YASL_Table_set(math, ceil_str, ceil_fn);
	struct YASL_Object *floor_str = YASL_LiteralString("floor");
	struct YASL_Object *floor_fn = YASL_CFunction(YASL_math_floor, 1);
	YASL_Table_set(math, floor_str, floor_fn);

	struct YASL_Object *deg_str = YASL_LiteralString("deg");
	struct YASL_Object *deg_fn = YASL_CFunction(YASL_math_deg, 1);
	YASL_Table_set(math, deg_str, deg_fn);
	struct YASL_Object *rad_str = YASL_LiteralString("rad");
	struct YASL_Object *rad_fn = YASL_CFunction(YASL_math_rad, 1);
	YASL_Table_set(math, rad_str, rad_fn);

	struct YASL_Object *isprime_str = YASL_LiteralString("isprime");
	struct YASL_Object *isprime_fn = YASL_CFunction(YASL_math_isprime, 1);
	YASL_Table_set(math, isprime_str, isprime_fn);
	struct YASL_Object *gcd_str = YASL_LiteralString("gcd");
	struct YASL_Object *gcd_fn = YASL_CFunction(YASL_math_gcd, 2);
	YASL_Table_set(math, gcd_str, gcd_fn);
	struct YASL_Object *lcm_str = YASL_LiteralString("lcm");
	struct YASL_Object *lcm_fn = YASL_CFunction(YASL_math_lcm, 2);
	YASL_Table_set(math, lcm_str, lcm_fn);

	struct YASL_Object *rand_str = YASL_LiteralString("rand");
	struct YASL_Object *rand_fn = YASL_CFunction(YASL_math_rand, 1);
	YASL_Table_set(math, rand_str, rand_fn);

	YASL_declglobal(S, "math");
	YASL_pushobject(S, math);
	YASL_setglobal(S, "math");

	free(abs_str);
	free(abs_fn);

	free(exp_str);
	free(exp_fn);

	free(log_str);
	free(log_fn);

	free(pi_str);
	free(pi_val);

	free(nan_str);
	free(nan_val);

	free(inf_str);
	free(inf_val);

	free(sqrt_str);
	free(sqrt_fn);

	free(cos_str);
	free(cos_fn);
	free(sin_str);
	free(sin_fn);
	free(tan_str);
	free(tan_fn);
	free(acos_str);
	free(acos_fn);
	free(asin_str);
	free(asin_fn);
	free(atan_str);
	free(atan_fn);

	free(ceil_str);
	free(ceil_fn);
	free(floor_str);
	free(floor_fn);

	free(deg_str);
	free(deg_fn);
	free(rad_str);
	free(rad_fn);

	free(isprime_str);
	free(isprime_fn);
	free(gcd_str);
	free(gcd_fn);
	free(lcm_str);
	free(lcm_fn);

	free(rand_str);
	free(rand_fn);

	// free(math);

	return YASL_SUCCESS;
}

