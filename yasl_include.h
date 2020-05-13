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

#if defined __GNUC__ || defined __clang__
#define YASL_WARN_UNUSED __attribute((warn_unused_result))
#else
#define YASL_WARN_UNUSED
#endif

#define MSG_SYNTAX_ERROR "SyntaxError: "
#define MSG_TYPE_ERROR "TypeError: "
#define MSG_VALUE_ERROR "ValueError: "

#define YASL_PRINT_ERROR(fmt, ...) fprintf(stderr, K_RED fmt K_END, __VA_ARGS__)
#define YASL_PRINT_ERROR_SYNTAX(fmt, ...) YASL_PRINT_ERROR(MSG_SYNTAX_ERROR fmt, __VA_ARGS__)
#define YASL_PRINT_ERROR_TYPE(fmt, ...) YASL_PRINT_ERROR(MSG_TYPE_ERROR fmt, __VA_ARGS__)

#define YASL_PRINT_ERROR_BAD_ARG_TYPE(name, position, expected, actual) \
YASL_PRINT_ERROR_TYPE("%s expected arg in position %d to be of type %s, got arg of type %s.\n",\
                      name,\
                      position,\
                      YASL_TYPE_NAMES[expected],\
                      YASL_TYPE_NAMES[actual]);

#define YASL_PRINT_ERROR_TOO_MANY_VAR(line) YASL_PRINT_ERROR("Too many variables in current scope (line %" PRI_SIZET ").\n", line)

#endif
