#pragma once

#include <stdio.h>
#include <stdlib.h>
//#include "../list/list.h"
#include "../YASL_string/YASL_string.h"

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
    Y_STR_W,
    Y_LIST,
    Y_LIST_W,
    Y_TABLE,
    Y_TABLE_W,
    Y_FILE,
    Y_FN,
    Y_BFN
} YASL_Types;

struct List_s;
struct Hash_s;

typedef struct YASL_Object {
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

YASL_Object YASL_Undef(void);
YASL_Object YASL_Float(double value);
YASL_Object YASL_Integer(int64_t value);
YASL_Object YASL_Boolean(int value);
YASL_Object YASL_String(String_t *str);
YASL_Object YASL_Table(struct Hash_s *ht);

int isfalsey(YASL_Object v);
YASL_Object isequal(YASL_Object a, YASL_Object b);
int print(YASL_Object a);
int yasl_type_equals(YASL_Types a, YASL_Types b);

void inc_ref(YASL_Object *v);
void dec_ref(YASL_Object *v);

const char *YASL_TYPE_NAMES[13];

#define ASSERT_TYPE(vm, expected_type, name) do {\
                    if (vm->stack[vm->sp].type != expected_type) {\
                        printf("%s(...) expected first argument of type %s, got %s.\n", \
                                name, YASL_TYPE_NAMES[expected_type], YASL_TYPE_NAMES[vm->stack[vm->sp].type] );\
                    }\
                } while(0)
