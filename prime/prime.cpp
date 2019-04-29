#include <stdlib.h>
#include <math.h>
#include "prime.h"

const int64_t PRIMES_UNDER_200[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199};

// returns 1 -- prime, 0 -- not prime, -1 -- undefined
int is_prime(const int64_t x) {
    if (x < 2) return -1;
    if (x < 4) return 1;
	int64_t i;
	for (i = 0; i < (int64_t)(sizeof(PRIMES_UNDER_200)/sizeof(int64_t)); i++) {
		if ((x % PRIMES_UNDER_200[i]) == 0) return (x == PRIMES_UNDER_200[i]);
	}
    for (i = 201; i <= floor(sqrt((double)x)); i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }
    return 1;
}

int64_t next_prime(int64_t x) {
    while (is_prime(x++) != 1) {}
    return x - 1;
}
