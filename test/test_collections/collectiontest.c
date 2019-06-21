#include "settest.h"
#include "test/test_compiler/yats.h"

#define RUN(test) __YASL_TESTS_FAILED__ |= test()

SETUP_YATS();

// NOTE: these tests depend on the endianess of the system, so they may fail on big endian systems.

////////////////////////////////////////////////////////////////////////////////

int collectiontest() {
	RUN(settest);
	return __YASL_TESTS_FAILED__;
}
