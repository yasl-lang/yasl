#include "userdata.h"

#include <stdlib.h>

UserData_t *ud_new(void *data, int tag) {
    UserData_t *ud = malloc(sizeof(UserData_t));
    ud->tag = tag;
    ud->rc = rc_new();
    //ud->mt = NULL;
    ud->data = data;
    return ud;
}

void ud_del_data(UserData_t *ud) {
    free(ud->data);
    // dec_ref(ud->mt);
}

void ud_del_rc(UserData_t *ud) {
    rc_del(ud->rc);
    free(ud);
}

void ud_del(UserData_t *ud) {
    free(ud->data);
    // dec_ref(ud->mt);
    rc_del(ud->rc);
    free(ud);
}
