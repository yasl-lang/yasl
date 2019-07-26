#include <data-structures/YASL_string.h>
#include "test/yats.h"
#include "data-structures/YASL_string.h"

SETUP_YATS();

#define str_new_cliteral(s) str_new_sized(strlen(s), s)

/// check that yasl_string_len returns correct length.
static void test_string_len(void) {
	String_t *string = str_new_sized(strlen("hello"), "hello");
	ASSERT_EQ(yasl_string_len(string), strlen("hello"));
}

static void test_string_tofloat(void) {
	String_t *string = str_new_cliteral("1.234");
	ASSERT_EQ(1.234, string_tofloat(string));
}

static void test_string_toint(void) {
	String_t *string = str_new_cliteral("1234");
	ASSERT_EQ(string_toint(string), 1234);
	string = str_new_cliteral("0xBABE");
	ASSERT_EQ(string_toint(string), 0xBABE);
	string = str_new_cliteral("0b1001");
	ASSERT_EQ(string_toint(string), 9);
}

int strtest(void) {
	test_string_len();
	test_string_tofloat();
	test_string_toint();
	return __YASL_TESTS_FAILED__;
}
