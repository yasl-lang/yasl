#include "listtest.h"
#include "test/test_compiler/yats.h"

SETUP_YATS();

// NOTE: these tests depend on the endianess of the system, so they may fail on big endian systems.

////////////////////////////////////////////////////////////////////////////////

int methodtest() {
	RUN(listtest);
	return __YASL_TESTS_FAILED__;
}
