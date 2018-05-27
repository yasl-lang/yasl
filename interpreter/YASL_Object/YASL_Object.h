#pragma once

#include <stdio.h>
#include <stdlib.h>
//#include "../list/list.h"
#include "../YASL_string/YASL_string.h"
#define FALSEY(v)  (v.type == UNDEF || (v.type == BOOL && v.value.ival == 0) || (v.type == STR8 && (v.value.sval)->length == 0))  // returns true iff v is a falsey value
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

struct List_s;
struct Hash_s;

typedef struct {
    YASL_Types type;
    union {
        int64_t ival;
        double dval;
        String_t *sval;
        struct List_s *lval;
        struct Hash_s *mval;
        FILE *fval;
    } value;
} YASL_Object;

typedef struct {
    char type;
    double value;
} FloatConstant;

YASL_Object isequal(YASL_Object a, YASL_Object b);
int print(YASL_Object a);

const char *YASL_TYPE_NAMES[64/8];