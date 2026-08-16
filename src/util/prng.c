#include <stdint.h>

#define STATE_SIZE 624
#define CONST_F 1812433253
#define BITMASK(n) (0xFFFFFFFF & (n))

// Initialize it statically, to prevent accidents if someone forgets to seed it.
static int32_t state[STATE_SIZE] = {
#include "prng.dat"
};
static int index = 0;

void mtwist_init(unsigned seed) {
	state[0] = seed;
	for (int i = 1; i < STATE_SIZE; i++) {
		state[i] = BITMASK(CONST_F * (state[i-1] ^ ((state[i-1]) >> 30)) + i);
	}
	index = 0;
}


static void mtwist_twist(void) {
	for (int i = 0; i < STATE_SIZE; i++) {
		int32_t y = (state[i] & 0x80000000) + (state[(i + 1) % STATE_SIZE]) & 0x7fffffff;
		state[i] = state[(i+397) % STATE_SIZE] ^ (y >> 1);
		if (y % 2) {
			state[i] ^= 0x9908b0df;
		}
	}
}

int32_t mtwist_rand(void) {
	if (index == 0) {
		mtwist_twist();
	}

	int32_t y = state[index++];
	y = y ^ (y >> 11);
	y = y ^ ((y << 6) & 0x9d2c5680);
	y = y ^ ((y << 15) & 0xefc60000);
	y = y ^ (y >> 18);

	index %= STATE_SIZE;
	return y;
}

int ya_rand() {
	return mtwist_rand();
}

void ya_srand(unsigned s) {
	mtwist_init(s);
}