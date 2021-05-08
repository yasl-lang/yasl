#ifndef YASL_USERDATA_H_
#define YASL_USERDATA_H_

#include "refcount.h"

struct YASL_Table;

struct RC_UserData {
	struct RC rc;        // DO NOT REARRANGE. RC MUST BE THE FIRST MEMBER OF THIS STRUCT.
	const char *tag;
	void (*destructor)(void *);
	struct RC_UserData *mt;
	void *data;
};

struct RC_UserData *ud_new(void *data, const char *tag, struct RC_UserData *mt, void (*destructor)(void *));
void ud_del_data(struct RC_UserData *ud);
void ud_del_rc(struct RC_UserData *ud);
void ud_del(struct RC_UserData *ud);

void ud_setmt(struct RC_UserData *ud, struct RC_UserData *mt);

#endif
