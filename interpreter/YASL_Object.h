#ifndef YASL_YASL_OBJECT_H_
#define YASL_YASL_OBJECT_H_

#include "data-structures/YASL_String.h"
#include "yasl_conf.h"
#include "yasl_types.h"
// #include "interpreter/closure.h"

#define UNDEF_C ((struct YASL_Object){ .type = Y_UNDEF, .value = { .ival = 0 }  })
#define FALSE_C ((struct YASL_Object){ .type = Y_BOOL, .value = {.ival = 0 }})
#define TRUE_C ((struct YASL_Object){ .type = Y_BOOL, .value = {.ival = 1 }})

#define YASL_END() ((struct YASL_Object){ .type = Y_END, .value = {.ival = 0}})
#define YASL_UNDEF() ((struct YASL_Object){ .type = Y_UNDEF, .value = {.ival = 0 }})
#define YASL_FLOAT(d) ((struct YASL_Object){ .type = Y_FLOAT, .value = {.dval = d }})
#define YASL_INT(i) ((struct YASL_Object){ .type = Y_INT, .value = {.ival = i }})
#define YASL_BOOL(b) ((struct YASL_Object){ .type = Y_BOOL, .value = {.ival = b }})
#define YASL_STR(s) ((struct YASL_Object){ .type = Y_STR, .value = {.sval = s }})
#define YASL_LIST(l) ((struct YASL_Object){ .type = Y_LIST, .value = {.uval = l }})
#define YASL_TABLE(t) ((struct YASL_Object){ .type = Y_TABLE, .value = {.uval = t }})
#define YASL_USERDATA(p) ((struct YASL_Object){ .type = Y_USERDATA, .value = {.uval = p }})
#define YASL_USERPTR(p) ((struct YASL_Object){ .type = Y_USERPTR, .value = {.pval = p }})
#define YASL_FN(f) ((struct YASL_Object){ .type = Y_FN, .value = {.fval = f }})
#define YASL_CFN(f, n) ((struct YASL_Object){ .type = Y_CFN, .value = {.cval = new_cfn(f, n) }})

#define YASL_ISUNDEF(v) ((v).type == Y_UNDEF)
#define YASL_ISFLOAT(v) ((v).type == Y_FLOAT)
#define YASL_ISINT(v) ((v).type == Y_INT)
#define YASL_ISNUM(v) (YASL_ISINT(v) || YASL_ISFLOAT(v))
#define YASL_ISBOOL(v) ((v).type == Y_BOOL)
#define YASL_ISSTR(v) ((v).type == Y_STR)
#define YASL_ISLIST(v) ((v).type == Y_LIST)
#define YASL_ISTABLE(v) ((v).type == Y_TABLE)
#define YASL_ISUSERDATA(v) ((v).type == Y_USERDATA)
#define YASL_ISUSERPTR(v) ((v).type == Y_USERPTR)
#define YASL_ISFN(v) ((v).type == Y_FN)
#define YASL_ISCLOSURE(v) ((v).type == Y_CLOSURE)
#define YASL_ISCFN(v) ((v).type == Y_CFN)

#define YASL_GETFLOAT(v) ((v).value.dval)
#define YASL_GETINT(v) ((v).value.ival)
#define YASL_GETNUM(v) (YASL_ISFLOAT(v) ? YASL_GETFLOAT(v) : YASL_GETINT(v))
#define YASL_GETBOOL(v) ((v).value.ival)
#define YASL_GETSTR(v) ((v).value.sval)
#define YASL_GETLIST(v) ((struct YASL_List *)((v).value.uval->data))
#define YASL_GETTABLE(v) ((struct YASL_Table *)((v).value.uval->data))
#define YASL_GETUSERDATA(v) ((v).value.uval)
#define YASL_GETUSERPTR(v) ((v).value.pval)
#define YASL_GETFN(v) ((v).value.fval)
#define YASL_GETCFN(v) ((v).value.cval)

struct YASL_State;
struct RC_UserData;
struct Closure;

struct YASL_Object {
	enum YASL_Types type;
	union {
		yasl_int ival;
		yasl_float dval;
		struct YASL_String *sval;
		struct RC_UserData *uval;
		struct CFunction *cval;
		struct Closure *lval;
		unsigned char *fval;
		void *pval;
	} value;
};

struct CFunction {
	struct RC *rc;
	int num_args;
	int (*value)(struct YASL_State *);
};

struct CFunction *new_cfn(int (*value)(struct YASL_State *), int num_args);
void cfn_del_rc(struct CFunction *cfn);
void cfn_del_data(struct CFunction *cfn);

struct YASL_Object *YASL_Undef(void);
struct YASL_Object *YASL_Float(yasl_float value);
struct YASL_Object *YASL_Integer(yasl_int value);
struct YASL_Object *YASL_Boolean(bool value);
struct YASL_Object *YASL_String(struct YASL_String *str);
struct YASL_Object *YASL_Table(void);
struct YASL_Object *YASL_UserPointer(void *userdata);
struct YASL_Object *YASL_Function(int64_t index);
struct YASL_Object *YASL_CFunction(int (*value)(struct YASL_State *), int num_args);

/*
 * Either both types are strings or both types are numerical. Otherwise error
 */
int yasl_object_cmp(struct YASL_Object a, struct YASL_Object b);

int isfalsey(struct YASL_Object v);
struct YASL_Object isequal(struct YASL_Object a, struct YASL_Object b);
int print(struct YASL_Object a);

void inc_ref(struct YASL_Object *v);
void dec_ref(struct YASL_Object *v);

#endif
