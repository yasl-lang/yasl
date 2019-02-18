#include "refcount.h"

#include <stdlib.h>

#include "YASL_Object.h"
#include "interpreter/list.h"
#include "hashtable/hashtable.h"


struct RC *rc_new(void) {
	struct RC *rc = malloc(sizeof(struct RC));
	rc->refs = 0;
	rc->weak_refs = 0;
	return rc;
}

void rc_del(struct RC *rc) {
	free(rc);
}

static void inc_weak_ref(struct YASL_Object *v) {
	//printf(K_GRN "inc_weak(%s): ", YASL_TYPE_NAMES[v->type]);
	//print(*v);
	//puts(K_END);
	switch (v->type) {
	case Y_STR_W:v->value.sval->rc->weak_refs++;
		break;
	case Y_LIST_W:v->value.uval->rc->weak_refs++;
		break;
	case Y_TABLE_W:v->value.uval->rc->weak_refs++;
		break;
	default:puts("NOt Implemented");
		break;
	}
}

static void inc_strong_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:v->value.sval->rc->refs++;
		break;
	case Y_LIST:
	case Y_TABLE:v->value.uval->rc->refs++;
		break;
	case Y_CFN:v->value.cval->rc->refs++;
		break;
	default:puts("NOt Implemented");
		break;
	}
}

void inc_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
	case Y_LIST:
	case Y_TABLE:
	case Y_CFN:inc_strong_ref(v);
		break;
	case Y_STR_W:
	case Y_LIST_W:
	case Y_TABLE_W:inc_weak_ref(v);
		break;
	default:break;
	}
}

static void dec_weak_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR_W:if (--(v->value.sval->rc->weak_refs) || v->value.sval->rc->refs) return;
		str_del_rc(v->value.sval);
		v->type = Y_UNDEF;
		break;
	case Y_LIST_W:if (--(v->value.uval->rc->weak_refs) || v->value.uval->rc->refs) return;
		ls_del_rc(v->value.uval);
		v->type = Y_UNDEF;
		break;
	case Y_TABLE_W:if (--(v->value.uval->rc->weak_refs) || v->value.uval->rc->refs) return;
		rcht_del_rc(v->value.uval);
		v->type = Y_UNDEF;
		break;
	default:puts("NoT IMPELemented");
		exit(EXIT_FAILURE);
	}
}

void dec_strong_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
		//printf(K_MAG "after : %d\n" K_END, v->value.sval->rc->refs - 1);
		if (--(v->value.sval->rc->refs)) return;
		str_del_data(v->value.sval);
		if (v->value.sval->rc->weak_refs) return;
		str_del_rc(v->value.sval);
		break;
	case Y_LIST:if (--(v->value.uval->rc->refs)) return;
		ls_del_data(v->value.uval);
		if (v->value.uval->rc->weak_refs) return;
		ls_del_rc(v->value.uval);
		break;
	case Y_TABLE:if (--(v->value.uval->rc->refs)) return;
		rcht_del_data(v->value.uval);
		if (v->value.uval->rc->weak_refs) return;
		rcht_del_rc(v->value.uval);
		break;
	case Y_CFN:if (--(v->value.cval->rc->refs)) return;
		cfn_del_data(v->value.cval);
		if (v->value.cval->rc->weak_refs) return;
		cfn_del_rc(v->value.cval);
		break;
	default:puts("NoT IMPELemented");
		exit(EXIT_FAILURE);
	}
}

void dec_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
	case Y_LIST:
	case Y_TABLE:
	case Y_CFN:dec_strong_ref(v);
		break;
	case Y_STR_W:
	case Y_LIST_W:
	case Y_TABLE_W:dec_weak_ref(v);
		break;
	default:break;
	}
}
