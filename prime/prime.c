#include <stdlib.h>
#include <math.h>
#include "prime.h"

const int64_t PRIMES_UNDER_200[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199};

// returns 1 -- prime, 0 -- not prime, -1 -- undefined
int is_prime(const int64_t x) {
    if (x < 2) return -1;
    if (x < 4) return 1;
	int i;
	for (i = 0; i < sizeof(PRIMES_UNDER_200)/sizeof(int64_t); i++) {
		if ((x % PRIMES_UNDER_200[i]) == 0) {
            return 0;
        }
	}
    for (i = 201; i <= floor(sqrt((double)x)); i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }
    return 1;
}

int64_t exp_mod_helper(int64_t base, int64_t exp, const int64_t p) {
	while (exp < 0) exp += (p-1);
	exp %= p-1;

	int64_t r = 1;
	int64_t i = 1;
	while (i <= exp && i != 0) {
		if ((exp & i) != 0) r = (r * base) % p;
		base = (base*base) % p; i <<= 1;
	}
	return r;
}
// returns 1 -- prime, 0 -- not prime, -1 -- undefined
int prob_is_prime(const int64_t x, const int64_t count) {
	// bounds..
	if (x < 2 || count < 1) return -1;

	// For small numbers, less is more
	if (x < 1000000) return is_prime(x);

	// Apply Miller-Rabin probablistic primality test
	int64_t r = 1;
	int64_t d = (x - 1) >> 1;
	while (d % 2 == 0) {r++; d >>= 1;}
	// Ensure 'a' can be squared without overflowing int64_t
	const int64_t max = 3037000498 < x-3 ? 3037000498 : x-3;
	for (int64_t i = 0; i < count; i++) {
		int64_t a = (rand() % max) + 2;
		int64_t b = exp_mod_helper(a, d, x);
		if (b == 1 || b == x - 1) continue;
		int64_t j = 0;
		for (; j < r-1; j++) {
			b = (b*b) % x;
			if (b == x - 1) break;
		}
		if (j == r - 1) return 0;
	}
	return 1;
}

int64_t next_prime(int64_t x) {
    while (is_prime(x++) != 1) {}
    return x - 1;
}
