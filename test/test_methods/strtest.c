#include "test/yats.h"
#include "interpreter/YASL_string.h"

SETUP_YATS();

/// check that list_append is resizing the list properly.
/// Does not check that correct elements are appended
static void teststrlen(void) {
	String_t *string = str_new_sized(strlen("hello"), "hello");
	ASSERT_EQ(yasl_string_len(string), strlen("hello"));
}



int strtest(void) {
	teststrlen();
	return __YASL_TESTS_FAILED__;
}
