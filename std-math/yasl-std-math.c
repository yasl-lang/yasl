#include "yasl-std-math.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "YASL_Object.h"

#define POP_NUMBER(state, obj_name, fn_name) \
					struct YASL_Object *obj_name = YASL_popobject(state); \
                    if (!YASL_ISNUM(*obj_name)) { \
                        printf("%s(...) expected first argument of numerical type, got %s.\n", \
                                fn_name, YASL_TYPE_NAMES[obj_name->type] ); \
						return -1; \
                    }

const double YASL_PI = 3.14159265358979323851280895940618620443274267017841339111328125;

int YASL_math_abs(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.abs");

	if (YASL_ISINT(*num)) {
		int64_t i = num->value.ival;
		if (i < 0) i = -i;
		return YASL_pushinteger(S, i);
	} else {
		double n = num->value.dval;
		if (n < 0) n = -n;
		return YASL_pushfloat(S, n);
	}
}

int YASL_math_exp(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.exp");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, exp(n));
}

int YASL_math_log(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.log");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, log(n));
}

int YASL_math_sqrt(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.sqrt");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, sqrt(n));
}

int YASL_math_cos(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.cos");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, cos(n));
}
int YASL_math_sin(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.sin");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, sin(n));
}
int YASL_math_tan(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.tan");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, tan(n));
}
int YASL_math_acos(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.acos");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, acos(n));
}
int YASL_math_asin(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.asin");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, asin(n));
}
int YASL_math_atan(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.atan");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, atan(n));
}

int YASL_math_ceil(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.ceil");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, ceil(n));
}
int YASL_math_floor(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.floor");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}
	return YASL_pushfloat(S, floor(n));
}

int YASL_math_deg(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.deg");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}

	n *= (double)180.0/YASL_PI;
	return YASL_pushfloat(S, n);
}
int YASL_math_rad(struct YASL_State *S) {
	POP_NUMBER(S, num, "math.rad");

	double n;
	if (YASL_ISINT(*num)) {
		n = num->value.ival;
	} else {
		n = num->value.dval;
	}

	n *= YASL_PI/(double)180.0;
	return YASL_pushfloat(S, n);
}

int YASL_load_math(struct YASL_State *S) {
    struct YASL_Object *math = YASL_Table();

    struct YASL_Object *abs_str = YASL_CString("abs");
    struct YASL_Object *abs_fn = YASL_CFunction(YASL_math_abs, 1);
    YASL_Table_set(math, abs_str, abs_fn);

    struct YASL_Object *exp_str = YASL_CString("exp");
    struct YASL_Object *exp_fn = YASL_CFunction(YASL_math_exp, 1);
    YASL_Table_set(math, exp_str, exp_fn);

	struct YASL_Object *log_str = YASL_CString("log");
    struct YASL_Object *log_fn = YASL_CFunction(YASL_math_log, 1);
    YASL_Table_set(math, log_str, log_fn);

	struct YASL_Object *pi_str = YASL_CString("pi");
    struct YASL_Object *pi_val = malloc(sizeof(struct YASL_Object));
	pi_val->type = Y_FLOAT;
	pi_val->value.dval = YASL_PI;
    YASL_Table_set(math, pi_str, pi_val);

	struct YASL_Object *sqrt_str = YASL_CString("sqrt");
    struct YASL_Object *sqrt_fn = YASL_CFunction(YASL_math_sqrt, 1);
    YASL_Table_set(math, sqrt_str, sqrt_fn);

	struct YASL_Object *cos_str = YASL_CString("cos");
    struct YASL_Object *cos_fn = YASL_CFunction(YASL_math_cos, 1);
    YASL_Table_set(math, cos_str, cos_fn);
	struct YASL_Object *sin_str = YASL_CString("sin");
    struct YASL_Object *sin_fn = YASL_CFunction(YASL_math_sin, 1);
    YASL_Table_set(math, sin_str, sin_fn);
	struct YASL_Object *tan_str = YASL_CString("tan");
    struct YASL_Object *tan_fn = YASL_CFunction(YASL_math_tan, 1);
    YASL_Table_set(math, tan_str, tan_fn);
	struct YASL_Object *acos_str = YASL_CString("acos");
    struct YASL_Object *acos_fn = YASL_CFunction(YASL_math_acos, 1);
    YASL_Table_set(math, acos_str, acos_fn);
	struct YASL_Object *asin_str = YASL_CString("asin");
    struct YASL_Object *asin_fn = YASL_CFunction(YASL_math_asin, 1);
    YASL_Table_set(math, asin_str, asin_fn);
	struct YASL_Object *atan_str = YASL_CString("atan");
    struct YASL_Object *atan_fn = YASL_CFunction(YASL_math_atan, 1);
    YASL_Table_set(math, atan_str, atan_fn);

	struct YASL_Object *ceil_str = YASL_CString("ceil");
    struct YASL_Object *ceil_fn = YASL_CFunction(YASL_math_ceil, 1);
    YASL_Table_set(math, ceil_str, ceil_fn);
	struct YASL_Object *floor_str = YASL_CString("floor");
    struct YASL_Object *floor_fn = YASL_CFunction(YASL_math_floor, 1);
    YASL_Table_set(math, floor_str, floor_fn);

	struct YASL_Object *deg_str = YASL_CString("deg");
    struct YASL_Object *deg_fn = YASL_CFunction(YASL_math_deg, 1);
    YASL_Table_set(math, deg_str, deg_fn);
	struct YASL_Object *rad_str = YASL_CString("rad");
    struct YASL_Object *rad_fn = YASL_CFunction(YASL_math_rad, 1);
    YASL_Table_set(math, rad_str, rad_fn);

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

    return YASL_SUCCESS;
}

