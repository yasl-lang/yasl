#ifndef YASL_YASL_CONF_H_
#define YASL_YASL_CONF_H_

#include <inttypes.h>

#if defined __GNUC__ || defined __clang__
#define YASL_DEPRECATE __attribute__((deprecated))
#else
#define YASL_DEPRECATE
#endif

// @@ yasl_float
// Which floating point type YASL will use.
#define yasl_float double

// @@ yasl_int
// Which integral type YASL will use.
#define yasl_int int64_t

// @@ STACK_SIZE
// How big the stack is for the YASL VM
#define STACK_SIZE 1024

#endif
