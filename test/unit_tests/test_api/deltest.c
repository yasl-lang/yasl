#include "yats.h"
#include "yasl.h"
#include "yasl_state.h"

SETUP_YATS();

static void testnull(void) {
	ASSERT_SUCCESS(YASL_delstate(NULL));
}

int deltest(void) {
	testnull();
	return __YASL_TESTS_FAILED__;
}
