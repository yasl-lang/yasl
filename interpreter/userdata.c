#include "userdata.h"

#include "interpreter/refcount.h"
#include "YASL_Object.h"

struct RC_UserData *ud_new(void *data, const char *name, struct RC_UserData *mt, void (*destructor)(void *)) {
	struct RC_UserData *ud = (struct RC_UserData *)malloc(sizeof(struct RC_UserData));
	//ud->tag = tag;
	ud->name = name;
	ud->rc = rc_new();
	ud->mt = mt;
	if (mt)	mt->rc->refs++;
	ud->destructor = destructor;
	ud->data = data;
	return ud;
}

void ud_del_data(struct RC_UserData *ud) {
	if (ud->mt) {
		struct YASL_Object v = YASL_TABLE(ud->mt);
		dec_ref(&v);
	}
	if (ud->destructor) ud->destructor(ud->data);
}

void ud_del_rc(struct RC_UserData *ud) {
	rc_del(ud->rc);
	free(ud);
}

void ud_del(struct RC_UserData *ud) {
	ud->destructor(ud->data);
	rc_del(ud->rc);
	free(ud);
}

void ud_setmt(struct RC_UserData *ud, struct RC_UserData *mt) {
	if (mt) mt->rc->refs++;
	if (ud->mt) {
		struct YASL_Object v = YASL_TABLE(ud->mt);
		dec_ref(&v);
	}
	ud->mt = mt;
}
