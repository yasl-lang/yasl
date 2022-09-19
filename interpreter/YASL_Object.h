#ifndef YASL_YASL_OBJECT_H_
#define YASL_YASL_OBJECT_H_

#include "data-structures/YASL_String.h"
#include "yasl_conf.h"
#include "yasl_types.h"
#include "yasl.h"

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

#define YASL_GETLIST(v) ((struct YASL_List *)((v).value.uval->data))
#define YASL_GETTABLE(v) ((struct YASL_Table *)((v).value.uval->data))
#define YASL_GETUSERDATA(v) ((v).value.uval)
#define YASL_GETUSERPTR(v) ((v).value.pval)
#define YASL_GETCFN(v) ((v).value.cval)

struct YASL_State;
struct RC_UserData;
struct Closure;

struct YASL_Object {
	enum YASL_Types type;
	union {
		yasl_int ival;             // bool or int
		yasl_float dval;           // float
		struct YASL_String *sval;  // str
		struct RC_UserData *uval;  // list, table, userdata
		struct CFunction *cval;    // C fn
		struct Closure *lval;      // closure
		unsigned char *fval;       // YASL fn
		void *pval;                // userptr
	} value;
};

struct CFunction {
	struct RC rc;
	int num_args;
	YASL_cfn value;
};

struct CFunction *new_cfn(YASL_cfn value, int num_args);
void cfn_del_rc(struct CFunction *cfn);
void cfn_del_data(struct CFunction *cfn);

struct YASL_Object *YASL_String(struct YASL_String *str);
struct YASL_Object *YASL_Table(void);

/*
 * Either both types are strings or both types are numerical. Otherwise error
 */
int yasl_object_cmp(struct YASL_Object a, struct YASL_Object b);

bool ishashable(const struct YASL_Object *const v);
bool isfalsey(const struct YASL_Object *const v);
bool isequal(const struct YASL_Object *const a, const struct YASL_Object *const b);
bool isequal_typed(const struct YASL_Object *const a, const struct YASL_Object *const b);

const char *obj_typename(const struct YASL_Object *const v);

inline bool obj_isundef(const struct YASL_Object *const v) {
	return v->type == Y_UNDEF;
}

inline bool obj_isfloat(const struct YASL_Object *const v) {
	return v->type == Y_FLOAT;
}

inline bool obj_isint(const struct YASL_Object *const v) {
	return v->type == Y_INT;
}

inline bool obj_isnum(const struct YASL_Object *const v) {
	return obj_isint(v) || obj_isfloat(v);
}


inline bool obj_isbool(const struct YASL_Object *const v) {
	return v->type == Y_BOOL;
}

inline bool obj_isstr(const struct YASL_Object *const v) {
	return v->type == Y_STR;
}

inline bool obj_islist(const struct YASL_Object *const v) {
	return v->type == Y_LIST;
}

inline bool obj_istable(const struct YASL_Object *const v) {
	return v->type == Y_TABLE;
}

inline bool obj_isuserdata(const struct YASL_Object *const v) {
	return v->type == Y_USERDATA;
}

inline bool obj_isuserptr(const struct YASL_Object *const v) {
	return v->type == Y_USERPTR;
}

inline bool obj_isfn(const struct YASL_Object *const v) {
	return v->type == Y_FN;
}

inline bool obj_isclosure(const struct YASL_Object *const v) {
	return v->type == Y_CLOSURE;
}

inline bool obj_iscfn(const struct YASL_Object *const v) {
	return v->type == Y_CFN;
}

inline bool obj_getbool(const struct YASL_Object *const v) {
	return (bool) v->value.ival;
}

inline yasl_float obj_getfloat(const struct YASL_Object *const v) {
	return v->value.dval;
}

inline yasl_int obj_getint(const struct YASL_Object *const v) {
	return v->value.ival;
}

inline yasl_float obj_getnum(const struct YASL_Object *const v) {
	return obj_isfloat(v) ? obj_getfloat(v) : obj_getint(v);
}

inline struct YASL_String *obj_getstr(const struct YASL_Object *const v) {
	return v->value.sval;
}

inline void *obj_getuserptr(const struct YASL_Object *const v) {
	return v->value.pval;
}

struct VM;

void inc_ref(struct YASL_Object *v);
void dec_ref(struct YASL_Object *v);
void dec_strong_ref(struct VM *vm, struct YASL_Object *v);

#endif
