#include "userdata.h"

#include <stdlib.h>

struct RC_UserData *ud_new(void *data, int tag, struct YASL_HashTable *mt, void (*destructor)(void *)) {
	struct RC_UserData *ud = (struct RC_UserData *)malloc(sizeof(struct RC_UserData));
	ud->tag = tag;
	ud->rc = rc_new();
	ud->mt = mt;
	ud->destructor = destructor;
	ud->data = data;
	return ud;
}

void ud_del_data(struct RC_UserData *ud) {
	if (ud->destructor) ud->destructor(ud->data);
}

void ud_del_rc(struct RC_UserData *ud) {
	rc_del(ud->rc);
	free(ud);
}

void ud_del(struct RC_UserData *ud) {
	ud->destructor(ud->data);
	// dec_ref(ud->mt);
	rc_del(ud->rc);
	free(ud);
}
