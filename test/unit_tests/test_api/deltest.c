#include "yats.h"
#include "yasl.h"
#include "yasl_state.h"

SETUP_YATS();

static void testnull(void) {
	ASSERT_SUCCESS(YASL_delstate(NULL));
}

int TEST(deltest) {
	testnull();
	return NUM_FAILED;
}
