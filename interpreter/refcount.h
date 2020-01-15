#ifndef YASL_REFCOUNT_H
#define YASL_REFCOUNT_H

#include <stdlib.h>

struct YASL_Object;

struct RC {
    size_t refs;
    size_t weak_refs;
};

struct RC *rc_new(void);
void rc_del(struct RC *rc);

void dec_ref(struct YASL_Object *v);

#endif
