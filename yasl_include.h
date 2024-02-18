#ifndef YASL_YASL_INCLUDE_H_
#define YASL_YASL_INCLUDE_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "yasl_conf.h"

// Ugly hack for MinGW, since it doesn't support %z
#ifdef _WIN32
#  ifdef _WIN64
#    define PRI_SIZET PRIu64
#  else
#    define PRI_SIZET PRIu32
#  endif
#else
#  define PRI_SIZET "zu"
#endif

#ifdef __EMSCRIPTEN__
#define K_END
#define K_RED
#define K_GRN
#define K_YEL
#define K_BLU
#define K_MAG
#define K_CYN
#define K_WHT
#else
#define K_END "\x1B[0m"
#define K_RED "\x1B[31m"
#define K_GRN "\x1B[32m"
#define K_YEL "\x1B[33m"
#define K_BLU "\x1B[34m"
#define K_MAG "\x1B[35m"
#define K_CYN "\x1B[36m"
#define K_WHT "\x1B[37m"
#endif

#define YASL_UNUSED(x) (void)x

#define MSG_SYNTAX_ERROR "SyntaxError: "
#define MSG_TYPE_ERROR "TypeError: "
#define MSG_VALUE_ERROR "ValueError: "

#endif
