#ifndef YASL_YASL_CONF_H_
#define YASL_YASL_CONF_H_

#include <inttypes.h>

#if defined __GNUC__ || defined __clang__
#define YASL_DEPRECATE __attribute__((deprecated))
#else
#define YASL_DEPRECATE
#endif

#if defined __GNUC__ || defined __clang__
#define YASL_FORMAT_CHECK __attribute__((format (printf, 2, 3)))
#else
#define YASL_FORMAT_CHECK
#endif

#if defined __GNUC__ || defined __clang__
#define YASL_NORETURN __attribute__((noreturn))
#elif defined _MSC_VER
#define YASL_NORETURN __declspec(noreturn)
#else
#define YASL_NORETURN
#endif

#if defined(WIN32) || defined(_WIN32)
#define YASL_USE_WIN
#elif defined(__unix__)
#define YASL_USE_UNIX
#elif defined(__APPLE__)
#define YASL_USE_APPLE
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
