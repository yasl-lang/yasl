#include <math.h>
#include "prime.h"

// returns 1 -- prime, 0 -- not prime, -1 -- undefined
int is_prime(const int64_t x) {
    if (x < 2) return -1;
    if (x < 4) return 1;
    if ((x % 2) == 0) return 0;
    int i;
    for (i = 3; i <= floor(sqrt((double)x)); i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }
    return 1;
}

int next_prime(int64_t x) {
    while (is_prime(x++) != 1) {}
    return x;
}
