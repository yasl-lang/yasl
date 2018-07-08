#pragma once

#include <stdio.h>
#include <stdlib.h>
//#include "../list/list.h"
#include "../YASL_string/YASL_string.h"
#define FALSEY(v)  (v.type == Y_UNDEF || (v.type == Y_BOOL && v.value.ival == 0) || (v.type == Y_STR && (v.value.sval)->end - v.value.sval->start == 0))  // returns true iff v is a falsey value
#define DVAL(v)  (*((double*)&v.value))
#define TRUE_C   ((YASL_Object) {Y_BOOL, 1})
#define FALSE_C  ((YASL_Object) {Y_BOOL, 0})
#define UNDEF_C  ((YASL_Object) {Y_UNDEF, 0})

//Keep up to date with the YASL_TYPE_NAMES
typedef enum {
    Y_END = -1,
    Y_UNDEF,
    Y_FLOAT64,
    Y_INT64,
    Y_BOOL,
    Y_STR,
    Y_LIST,
    Y_TABLE,
    Y_FILE,
    Y_FN,
    Y_BFN
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

const char *YASL_TYPE_NAMES[10];

#define ASSERT_TYPE(vm, expected_type, name) do {\
                    if (vm->stack[vm->sp].type != expected_type) {\
                        printf("%s(...) expected first argument of type %s, got %s.\n", \
                                name, YASL_TYPE_NAMES[expected_type], YASL_TYPE_NAMES[vm->stack[vm->sp].type] );\
                    }\
                } while(0)
