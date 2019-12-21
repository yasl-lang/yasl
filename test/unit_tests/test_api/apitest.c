#include "test/yats.h"
#include "pushtest.h"
#include "poptest.h"

SETUP_YATS();

////////////////////////////////////////////////////////////////////////////////

int apitest() {
	RUN(pushtest);
	RUN(poptest);
	return __YASL_TESTS_FAILED__;
}
