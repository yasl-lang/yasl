#include "test/yats.h"
#include "pushtest.h"
#include "poptest.h"
#include "deltest.h"

SETUP_YATS();

////////////////////////////////////////////////////////////////////////////////

int apitest() {
	RUN(pushtest);
	RUN(poptest);
	RUN(deltest);
	return __YASL_TESTS_FAILED__;
}
