#include "refcount.h"

#include <stdio.h>

#include "data-structures/YASL_List.h"
#include "data-structures/YASL_Table.h"
#include "YASL_Object.h"
#include "interpreter/closure.h"

struct RC *rc_new(void) {
	struct RC *rc = (struct RC *)malloc(sizeof(struct RC));
	rc->refs = 0;
	rc->weak_refs = 0;
	return rc;
}

void rc_del(struct RC *rc) {
	free(rc);
}

static void inc_weak_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR_W:
		v->value.sval->rc->weak_refs++;
		break;
	case Y_LIST_W:
		v->value.uval->rc->weak_refs++;
		break;
	case Y_TABLE_W:
		v->value.uval->rc->weak_refs++;
		break;
	default:
		/* do nothing */
		break;
	}
}

static void inc_strong_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
		v->value.sval->rc->refs++;
		break;
	case Y_USERDATA:
	case Y_LIST:
	case Y_TABLE:
		v->value.uval->rc->refs++;
		break;
	case Y_CFN:
		v->value.cval->rc->refs++;
		break;
	case Y_CLOSURE:
		v->value.lval->rc->refs++;
		break;
	default:
		/* do nothing */
		break;
	}
}

void inc_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
	case Y_LIST:
	case Y_TABLE:
	case Y_USERDATA:
	case Y_CFN:
	case Y_CLOSURE:
		inc_strong_ref(v);
		break;
	case Y_STR_W:
	case Y_LIST_W:
	case Y_TABLE_W:
		inc_weak_ref(v);
		break;
	default:
		break;
	}
}

static void dec_weak_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR_W:
		if (--(v->value.sval->rc->weak_refs) || v->value.sval->rc->refs) return;
		str_del_rc(v->value.sval);
		v->type = Y_UNDEF;
		break;
	case Y_LIST_W:
		if (--(v->value.uval->rc->weak_refs) || v->value.uval->rc->refs) return;
		ud_del_rc(v->value.uval);
		v->type = Y_UNDEF;
		break;
	case Y_TABLE_W:
		if (--(v->value.uval->rc->weak_refs) || v->value.uval->rc->refs) return;
		ud_del_rc(v->value.uval);
		v->type = Y_UNDEF;
		break;
	default:
		/* do nothing */
		break;
	}
}

void dec_strong_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
		if (--(v->value.sval->rc->refs)) return;
		str_del_data(v->value.sval);
		if (v->value.sval->rc->weak_refs) return;
		str_del_rc(v->value.sval);
		v->type = Y_UNDEF;
		break;
	case Y_LIST:
	case Y_USERDATA:
	case Y_TABLE:
		if (--(v->value.uval->rc->refs)) return;
		ud_del_data(v->value.uval);
		if (v->value.uval->rc->weak_refs) return;
		ud_del_rc(v->value.uval);
		v->type = Y_UNDEF;
		break;
	case Y_CFN:
		if (--(v->value.cval->rc->refs)) return;
		cfn_del_data(v->value.cval);
		if (v->value.cval->rc->weak_refs) return;
		cfn_del_rc(v->value.cval);
		v->type = Y_UNDEF;
		break;
	case Y_CLOSURE:
		if (--(v->value.lval->rc->refs)) return;
		closure_del_data(v->value.lval);
		if (v->value.lval->rc->weak_refs) return;
		closure_del_rc(v->value.lval);
		v->type = Y_UNDEF;
		break;
	default:
		/* do nothing */
		break;
	}
}

void dec_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
	case Y_LIST:
	case Y_TABLE:
	case Y_USERDATA:
	case Y_CFN:
	case Y_CLOSURE:
		dec_strong_ref(v);
		break;
	case Y_STR_W:
	case Y_LIST_W:
	case Y_TABLE_W:
		dec_weak_ref(v);
		break;
	default:break;
	}
}
