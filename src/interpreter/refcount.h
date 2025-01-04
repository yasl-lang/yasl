#ifndef YASL_REFCOUNT_H_
#define YASL_REFCOUNT_H_

#include <stdlib.h>

#include <stdbool.h>

struct YASL_Object;

struct RC {
	size_t refs;
	bool is_condemned;
};

#define NEW_RC() ((struct RC) { 0, false })

#endif
