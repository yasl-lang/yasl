#include <interpreter/YASL_Object.h>
#include "settest.h"
#include "test/test_compiler/yats.h"
#include "std-collections/set.h"

SETUP_YATS();


static void testsearchset(void) {
	struct Set *set = set_new();
	set_insert(set, YASL_INT(1));
	set_insert(set, YASL_INT(2));
	set_insert(set, YASL_INT(3));
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(1))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(3))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(2))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(0))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(4))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(6))), false);
	ASSERT_EQ(set_length(set), 3);
}

static void testunionset(void) {
	struct Set *left = set_new();
	set_insert(left, YASL_INT(1));
	set_insert(left, YASL_INT(2));
	set_insert(left, YASL_INT(3));
	ASSERT_EQ(set_length(left), 3);
	struct Set *right = set_new();
	set_insert(right, YASL_INT(1));
	set_insert(right, YASL_INT(6));
	set_insert(right, YASL_INT(4));
	ASSERT_EQ(set_length(right), 3);
	struct Set *set = set_union(left, right);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(1))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(3))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(2))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(0))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(4))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(6))), true);
	ASSERT_EQ(set_length(set), 5);
}

static void testintersectionset(void) {
	struct Set *left = set_new();
	set_insert(left, YASL_INT(1));
	set_insert(left, YASL_INT(2));
	set_insert(left, YASL_INT(3));
	ASSERT_EQ(set_length(left), 3);
	struct Set *right = set_new();
	set_insert(right, YASL_INT(1));
	set_insert(right, YASL_INT(6));
	set_insert(right, YASL_INT(4));
	ASSERT_EQ(set_length(right), 3);
	struct Set *set = set_intersection(left, right);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(1))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(3))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(2))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(0))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(4))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(6))), false);
	ASSERT_EQ(set_length(set), 1);
}

static void testsymmetricdifferenceset(void) {
	struct Set *left = set_new();
	set_insert(left, YASL_INT(1));
	set_insert(left, YASL_INT(2));
	set_insert(left, YASL_INT(3));
	ASSERT_EQ(set_length(left), 3);
	struct Set *right = set_new();
	set_insert(right, YASL_INT(1));
	set_insert(right, YASL_INT(6));
	set_insert(right, YASL_INT(4));
	ASSERT_EQ(set_length(right), 3);
	struct Set *set = set_symmetric_difference(left, right);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(1))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(3))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(2))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(0))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(4))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(6))), true);
	ASSERT_EQ(set_length(set), 4);
}

static void testdifferenceset(void) {
	struct Set *left = set_new();
	set_insert(left, YASL_INT(1));
	set_insert(left, YASL_INT(2));
	set_insert(left, YASL_INT(3));
	ASSERT_EQ(set_length(left), 3);
	struct Set *right = set_new();
	set_insert(right, YASL_INT(1));
	set_insert(right, YASL_INT(6));
	set_insert(right, YASL_INT(4));
	ASSERT_EQ(set_length(left), 3);
	struct Set *set = set_difference(left, right);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(1))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(3))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(2))), true);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(0))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(4))), false);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(6))), false);
	ASSERT_EQ(set_length(set), 2);
}

static void testremoveset(void) {
	struct Set *set = set_new();
	set_insert(set, YASL_INT(1));
	ASSERT_EQ(set_length(set), 1);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(1))), true);
	set_rm(set, YASL_INT(1));
	ASSERT_EQ(set_length(set), 0);
	ASSERT_EQ(YASL_GETBOOL(set_search(set, YASL_INT(1))), false);
}

int settest(void) {
	testsearchset();
	testunionset();
	testintersectionset();
	testsymmetricdifferenceset();
	testdifferenceset();
	testremoveset();
	return __YASL_TESTS_FAILED__;
}
