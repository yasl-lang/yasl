

#include <inttypes.h>

#include "prime/prime.h"
#include "interpreter/YASL_Object.h"
#include "YASL_list.h"

size_t hash_function(const struct YASL_Object s, const size_t a, const size_t m);
size_t get_hash(const struct YASL_Object s, const size_t num_buckets, const size_t attempt);


