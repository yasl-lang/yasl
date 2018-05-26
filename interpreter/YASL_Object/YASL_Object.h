#pragma once

#include <stdio.h>
#include <stdlib.h>
//#include <interpreter/list/list.h>
#include "../YASL_string/YASL_string.h"
#define FALSEY(v)  (v.type == UNDEF || (v.type == BOOL && v.value == 0) || (v.type == STR8 && ((String_t*)v.value)->length == 0))  // returns true iff v is a falsey value
#define DVAL(v)  (*((double*)&v.value))
#define TRUE_C   ((YASL_Object) {BOOL, 1})
#define FALSE_C  ((YASL_Object) {BOOL, 0})
#define UNDEF_C  ((YASL_Object) {UNDEF, 0})

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
    /*union {
        int64_t ival;
        double dval;
        String_t *sval;
        List_t *lval;
        Hash_t *mval;
        FILE *fval;
    }; */
} YASL_Object;

typedef struct {
    char type;
    double value;
} FloatConstant;

YASL_Object isequal(YASL_Object a, YASL_Object b);
int print(YASL_Object a);

const char *YASL_TYPE_NAMES[64/8];