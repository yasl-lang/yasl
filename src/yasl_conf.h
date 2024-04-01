#ifndef YASL_YASL_CONF_H_
#define YASL_YASL_CONF_H_

#include <inttypes.h>

#if defined __GNUC__ || defined __clang__
#define YASL_DEPRECATED __attribute__((deprecated))
#elif defined _MSC_VER
#define YASL_DEPRECATED __declspec(deprecated)
#else
#define YASL_DEPRECATED
#endif

#define YASL_DEPRECATE YASL_DEPRECATED

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

#if defined __GNUC__ || defined __clang__
#define YASL_WARN_UNUSED __attribute((warn_unused_result))
#else
#define YASL_WARN_UNUSED
#endif  // defined __GNUC__ || defined __clang__

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

// @@ YASL_PATH_SEP
// What to use to separate paths.
#define YASL_PATH_SEP ';'

// @@ YASL_PATH_MARK
// What to use to use to mark the substitution in the path.
#define YASL_PATH_MARK '?'

// @@ YASL_DEFAULT_CPATH
// Where to search for C modules.
#if defined(YASL_USE_UNIX) || defined(YASL_USE_APPLE)
#define YASL_DEFAULT_CPATH "/usr/local/lib/yasl/lib?.so;" "/usr/local/lib/yasl/?.so;" "./lib?.so;" "./?.so;"
#define YASL_DEFAULT_PATH "/usr/local/lib/yasl/?.yasl;" "./?.yasl;"
#endif

#ifndef YASL_DEFAULT_CPATH
#define YASL_DEFAULT_CPATH ""
#endif

#ifndef YASL_DEFAULT_PATH
#define YASL_DEFAULT_PATH ""
#endif

#endif
