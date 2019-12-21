#include "yats.h"
#include "yasl.h"

SETUP_YATS();

int poptest(void) {
	return __YASL_TESTS_FAILED__;
}
