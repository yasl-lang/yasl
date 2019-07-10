#include "test/yats.h"
#include "interpreter/YASL_string.h"

SETUP_YATS();

/// check that yasl_string_len returns correct length.
static void teststrlen(void) {
	String_t *string = str_new_sized(strlen("hello"), "hello");
	ASSERT_EQ(yasl_string_len(string), strlen("hello"));
}


int strtest(void) {
	teststrlen();
	return __YASL_TESTS_FAILED__;
}
