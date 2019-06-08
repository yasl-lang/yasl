#include <stdlib.h>
#include <math.h>
#include "prime.h"
#include <inttypes.h>

const unsigned PRIMES_UNDER_200[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199};

// returns 1 -- prime, 0 -- not prime or undefined
bool is_prime(const size_t x) {
	if (x < 2) return false;
	if (x < 4) return true;
	for (int64_t i = 0; i < (int64_t) (sizeof(PRIMES_UNDER_200) / sizeof(int64_t)); i++) {
		if ((x % PRIMES_UNDER_200[i]) == 0) return (x == PRIMES_UNDER_200[i]);
	}
	for (int64_t i = 201; i <= floor(sqrt((double) x)); i += 2) {
		if ((x % i) == 0) {
			return false;
		}
	}
	return true;
}

size_t next_prime(size_t x) {
	while (!is_prime(x++)) {}
	return x - 1;
}
