#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "../YASL_string/YASL_string.h"
#define FALSEY(v)  (v.type == UNDEF || (v.type == BOOL && v.value == 0) || (v.type == STR8 && ((String_t*)v.value)->length == 0))  // returns true iff v is a falsey value
#define DVAL(v)  (*((double*)&v.value))
#define TRUE_C   ((Constant) {BOOL, 1})
#define FALSE_C  ((Constant) {BOOL, 0})
#define UNDEF_C  ((Constant) {UNDEF, 0})

enum types {
    UNDEF   = 0x00,
    FLOAT64 = 0x13,
    INT64   = 0x1B,
    BOOL    = 0x20,
    STR8    = 0x32,
    LIST    = 0x44,
    MAP     = 0x48,
    FILEH   = 0x50,
};

typedef struct {
    char type;
    int64_t value;
} Constant;

typedef struct {
    char type;
    double value;
} FloatConstant;

Constant isequal(Constant a, Constant b);
int print(Constant a);