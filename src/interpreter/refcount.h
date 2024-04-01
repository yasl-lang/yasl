#ifndef YASL_REFCOUNT_H_
#define YASL_REFCOUNT_H_

#include <stdlib.h>

struct YASL_Object;

struct RC {
	size_t refs;
};

#define NEW_RC() ((struct RC) { 0 })

#endif
