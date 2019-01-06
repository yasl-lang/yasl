#pragma once

#include <inttypes.h>

#define PRIME_A 37
#define PRIME_B 67

int is_prime(const int64_t x);
int64_t next_prime(int64_t x);
