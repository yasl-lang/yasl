#ifndef YASL_USERDATA_H_
#define YASL_USERDATA_H_

#include "refcount.h"

struct YASL_Table;
struct YASL_State;
struct VM;

struct RC_UserData {
	struct RC rc;        // DO NOT REARRANGE. RC MUST BE THE FIRST MEMBER OF THIS STRUCT.
	const char *tag;
	void (*destructor)(struct YASL_State *, void *);
	struct RC_UserData *mt;
	void *data;
};

struct RC_UserData *ud_new(void *data, const char *tag, struct RC_UserData *mt, void (*destructor)(struct YASL_State *, void *));
void ud_del_data(struct VM *vm, struct RC_UserData *ud);
void ud_del_rc(struct RC_UserData *ud);

void ud_setmt(struct VM *vm, struct RC_UserData *ud, struct RC_UserData *mt);

#endif
