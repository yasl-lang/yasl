#pragma once

#include <stdio.h>
#include <stdlib.h>
//#include "../list/list.h"
#include "../YASL_string/YASL_string.h"

#define UNDEF_C ((struct YASL_Object) { .type = Y_UNDEF, .value.ival = 0 })
#define FALSE_C ((struct YASL_Object) { .type = Y_BOOL, .value.ival = 0 })
#define TRUE_C ((struct YASL_Object) { .type = Y_BOOL, .value.ival = 1 })

#define YASL_END() ((struct YASL_Object) { .type = Y_END })
#define YASL_UNDEF() ((struct YASL_Object) { .type = Y_UNDEF })
#define YASL_FLOAT(d) ((struct YASL_Object) { .type = Y_FLOAT64, .value.dval = d })
#define YASL_INT(i) ((struct YASL_Object) { .type = Y_INT64, .value.ival = i })
#define YASL_BOOL(b) ((struct YASL_Object) { .type = Y_BOOL, .value.ival = b })
#define YASL_STR(s) ((struct YASL_Object) { .type = Y_STR, .value.sval = s })
#define YASL_LIST(l) ((struct YASL_Object) { .type = Y_LIST, .value.lval = l })
#define YASL_TBL(t) ((struct YASL_Object) { .type = Y_TABLE, .value.mval = t })
#define YASL_USERDATA(p) ((struct YASL_Object) { .type = Y_USERDATA, .value.uval = p })
#define YASL_USERPTR(p) ((struct YASL_Object) { .type = Y_USERPTR, .value.pval = p })
#define YASL_FN(f) ((struct YASL_Object) { .type = Y_FN, .value.ival = f })
#define YASL_CFN(f, n) ((struct YASL_Object) { .type = Y_CFN, .value.cval = new_cfn(f, n) })

struct YASL_State;

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
    Y_FN,
    Y_CFN,
    Y_USERPTR,
    Y_USERDATA,
    Y_USERDATA_W,
} YASL_Types;

struct List_s;
struct Hash_s;
struct UserData_s;
struct CFunction_s {
    RefCount *rc;
    int num_args;
    int (*value)(struct YASL_State *);
};

struct CFunction_s *new_cfn(int (*value)(struct YASL_State *), int num_args);
void cfn_del_rc(struct CFunction_s *cfn);
void cfn_del_data(struct CFunction_s *cfn);

struct YASL_Object {
    YASL_Types type;
    union {
        int64_t ival;
        double dval;
        String_t *sval;
        struct List_s *lval;
        struct Hash_s *mval;
        struct UserData_s *uval;
        struct CFunction_s *cval;
        void *pval;
    } value;
};

struct YASL_Object *YASL_Undef(void);
struct YASL_Object *YASL_Float(double value);
struct YASL_Object *YASL_Integer(int64_t value);
struct YASL_Object *YASL_Boolean(int value);
struct YASL_Object *YASL_String(String_t *str);
struct YASL_Object *YASL_Table(void);
struct YASL_Object *YASL_UserPointer(void *userdata);
struct YASL_Object *YASL_Function(int64_t index);
struct YASL_Object *YASL_CFunction(int (*value)(struct YASL_State *), int num_args);

int isfalsey(struct YASL_Object v);
struct YASL_Object isequal(struct YASL_Object a, struct YASL_Object b);
int print(struct YASL_Object a);
int yasl_type_equals(YASL_Types a, YASL_Types b);

void inc_ref(struct YASL_Object *v);
void dec_ref(struct YASL_Object *v);

const char *YASL_TYPE_NAMES[15];

#define ASSERT_TYPE(vm, expected_type, name) do {\
                    if (vm->stack[vm->sp].type != expected_type) {\
                        printf("%s(...) expected first argument of type %s, got %s.\n", \
                                name, YASL_TYPE_NAMES[expected_type], YASL_TYPE_NAMES[vm->stack[vm->sp].type] );\
                    }\
                } while(0)
