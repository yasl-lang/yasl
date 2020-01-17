#ifndef YASL_USERDATA_H_
#define YASL_USERDATA_H_

struct YASL_Table;
struct RC;

struct RC_UserData {
	struct RC *rc;        // DO NOT REARRANGE. RC MUST BE THE FIRST MEMBER OF THIS STRUCT.
	int tag;
	void (*destructor)(void *);
	struct YASL_Table *mt;
	void *data;
};


struct RC_UserData *ud_new(void *data, int tag, struct YASL_Table *mt, void (*destructor)(void *));
void ud_del_data(struct RC_UserData *ud);
void ud_del_rc(struct RC_UserData *ud);
void ud_del(struct RC_UserData *ud);

#endif
