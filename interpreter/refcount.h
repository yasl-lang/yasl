#ifndef YASL_REFCOUNT_H_
#define YASL_REFCOUNT_H_

#include <stdlib.h>

struct YASL_Object;

struct RC {
	size_t refs;
	size_t weak_refs;
};

#define NEW_RC() ((struct RC) { 0, 0 })

void dec_ref(struct YASL_Object *v);

#endif
