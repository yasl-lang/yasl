#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "../YASL_string/YASL_string.h"
#define FALSEY(v)  (v.type == UNDEF || (v.type == BOOL && v.value == 0) || (v.type == STR8 && ((String_t*)v.value)->length == 0))  // returns true iff v is a falsey value
#define DVAL(v)  (*((double*)&v.value))
#define TRUE_C   ((Constant) {BOOL, 1})
#define FALSE_C  ((Constant) {BOOL, 0})
#define UNDEF_C  ((Constant) {UNDEF, 0})

//Keep up to date with the YASL_TYPE_NAMES
typedef enum {
    UNDEF,
    FLOAT64,
    INT64,
    BOOL,
    STR8,
    LIST,
    MAP,
    FILEH
} YASL_Types;

typedef struct {
    YASL_Types type;
    int64_t value;
} Constant;

typedef Constant YASL_Object;

typedef struct {
    char type;
    double value;
} FloatConstant;

Constant isequal(Constant a, Constant b);
int print(Constant a);

const char *YASL_TYPE_NAMES[64/8];