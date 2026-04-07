#ifndef YASL_YASL_UTIL_GLOB_H_
#define YASL_YASL_UTIL_GLOB_H_

#include <stdbool.h>

#include "data-structures/LString.h"

bool glob(const char *pattern, const char *str, bool *no_error);
bool globL(struct LString pattern, struct LString str, bool *no_error);

#endif  // YASL_YASL_UTIL_GLOB_H_