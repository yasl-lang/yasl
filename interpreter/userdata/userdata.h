#pragma once

#include "refcount.h"

struct RC_Table;

typedef struct UserData_s {
    RefCount *rc;
    int tag;
    //struct RC_Table *mt;
    void *data;
} UserData_t;


UserData_t *ud_new(void *data, int tag);
void ud_del_data(UserData_t *ud);
void ud_del_rc(UserData_t *ud);
void ud_del(UserData_t *ud);