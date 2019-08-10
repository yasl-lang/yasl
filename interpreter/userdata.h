#pragma once

struct YASL_HashTable;
struct RC;

struct RC_UserData {
	struct RC *rc;        // DO NOT REARRANGE. RC MUST BE THE FIRST MEMBER OF THIS STRUCT.
	int tag;
	void (*destructor)(void *);
	struct YASL_HashTable *mt;
	void *data;
};


struct RC_UserData *ud_new(void *data, int tag, struct YASL_HashTable *mt, void (*destructor)(void *));
void ud_del_data(struct RC_UserData *ud);
void ud_del_rc(struct RC_UserData *ud);
void ud_del(struct RC_UserData *ud);
