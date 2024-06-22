#include "userdata.h"

#include "interpreter/refcount.h"
#include "VM.h"
#include "YASL_Object.h"

struct RC_UserData *ud_new(void *data, const char *tag, struct RC_UserData *mt, void (*destructor)(struct YASL_State *,void *)) {
	struct RC_UserData *ud = (struct RC_UserData *)malloc(sizeof(struct RC_UserData));
	ud->tag = tag;
	ud->rc = NEW_RC();
	ud->mt = mt;
	if (mt)	mt->rc.refs++;
	ud->destructor = destructor;
	ud->data = data;
	return ud;
}

void ud_del_data(struct VM *vm, struct RC_UserData *ud) {
	if (ud->mt) {
		struct YASL_Object v = YASL_TABLE(ud->mt);
		vm_dec_ref(vm, &v);
	}
	if (ud->destructor) ud->destructor((struct YASL_State *)vm, ud->data);
}

void ud_del_rc(struct RC_UserData *ud) {
	free(ud);
}

void ud_setmt(struct VM *vm, struct RC_UserData *ud, struct RC_UserData *mt) {
	if (mt) mt->rc.refs++;
	if (ud->mt) {
		struct YASL_Object v = YASL_TABLE(ud->mt);
		vm_dec_ref(vm, &v);
	}
	ud->mt = mt;
}
