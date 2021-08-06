#include "refcount.h"

#include <stdio.h>

#include "data-structures/YASL_List.h"
#include "data-structures/YASL_Table.h"
#include "YASL_Object.h"
#include "interpreter/closure.h"

static void inc_strong_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
		v->value.sval->rc.refs++;
		break;
	case Y_USERDATA:
	case Y_LIST:
	case Y_TABLE:
		v->value.uval->rc.refs++;
		break;
	case Y_CFN:
		v->value.cval->rc.refs++;
		break;
	case Y_CLOSURE:
		v->value.lval->rc.refs++;
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
	default:
		/* do nothing */
		break;
	}
}

void dec_strong_ref(struct YASL_Object *v) {
	switch (v->type) {
	case Y_STR:
		if (--(v->value.sval->rc.refs)) return;
		str_del_data(v->value.sval);
		str_del_rc(v->value.sval);
		v->type = Y_UNDEF;
		break;
	case Y_LIST:
	case Y_USERDATA:
	case Y_TABLE:
		if (--(v->value.uval->rc.refs)) return;
		ud_del_data(v->value.uval);
		ud_del_rc(v->value.uval);
		v->type = Y_UNDEF;
		break;
	case Y_CFN:
		if (--(v->value.cval->rc.refs)) return;
		cfn_del_data(v->value.cval);
		cfn_del_rc(v->value.cval);
		v->type = Y_UNDEF;
		break;
	case Y_CLOSURE:
		if (--(v->value.lval->rc.refs)) return;
		closure_del_data(v->value.lval);
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
	default:
		/* do nothing */
		break;
	}
}
