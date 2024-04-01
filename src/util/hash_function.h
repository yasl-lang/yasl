#ifndef YASL_HASH_FUNCTION_H_
#define YASL_HASH_FUNCTION_H_

#include <stdlib.h>

#include "interpreter/YASL_Object.h"

size_t hash_function(const struct YASL_Object s, const size_t a, const size_t m);
size_t get_hash(const struct YASL_Object s, const size_t num_buckets, const size_t attempt);

#endif
