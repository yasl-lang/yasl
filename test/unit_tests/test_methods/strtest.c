#include <data-structures/YASL_String.h>
#include "test/yats.h"
#include "data-structures/YASL_String.h"

SETUP_YATS();

#define str_new_cliteral(s) YASL_String_new_copyz_unbound(s)

/// check that YASL_String_len returns correct length.
static void test_string_len(void) {
	struct YASL_String *string = str_new_cliteral("hello");
	ASSERT_EQ(YASL_String_len(string), strlen("hello"));
	str_del(string);
}

static void test_string_tofloat(void) {
	ASSERT_EQ(1.234, YASL_String_tofloat("1.234", strlen("1.234")));
}

static void test_string_toint(void) {
	ASSERT_EQ(YASL_String_toint("1234", strlen("1234")), 1234);
	ASSERT_EQ(YASL_String_toint("0xBABE", strlen("0xBABE")), 0xBABE);
	ASSERT_EQ(YASL_String_toint("0b1001", strlen("0b1001")), 9);
}

TEST(strtest) {
	test_string_len();
	test_string_tofloat();
	test_string_toint();
	return NUM_FAILED;
}
