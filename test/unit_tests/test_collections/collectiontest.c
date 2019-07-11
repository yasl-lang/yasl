#include "settest.h"
#include "test/yats.h"

SETUP_YATS();

// NOTE: these tests depend on the endianess of the system, so they may fail on big endian systems.

////////////////////////////////////////////////////////////////////////////////

int collectiontest() {
	RUN(settest);
	return __YASL_TESTS_FAILED__;
}
