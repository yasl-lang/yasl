#ifndef YASL_PRIME_H_
#define YASL_PRIME_H_

#include "yasl_include.h"

#include <inttypes.h>
#include <stdlib.h>

#define PRIME_A 37
#define PRIME_B 67

bool is_prime(const size_t x);
size_t next_prime(size_t x);

#endif
