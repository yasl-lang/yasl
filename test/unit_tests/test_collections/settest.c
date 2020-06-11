#include <interpreter/YASL_Object.h>
#include "settest.h"
#include "test/yats.h"
#include "data-structures/YASL_Set.h"

SETUP_YATS();


static void testsearchset(void) {
	struct YASL_Set *set = YASL_Set_new();
	YASL_Set_insert(set, YASL_INT(1));
	YASL_Set_insert(set, YASL_INT(2));
	YASL_Set_insert(set, YASL_INT(3));
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(1))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(3))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(2))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(0))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(4))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(6))), false);
	ASSERT_EQ(YASL_Set_length(set), 3);
}

static void testunionset(void) {
	struct YASL_Set *left = YASL_Set_new();
	YASL_Set_insert(left, YASL_INT(1));
	YASL_Set_insert(left, YASL_INT(2));
	YASL_Set_insert(left, YASL_INT(3));
	ASSERT_EQ(YASL_Set_length(left), 3);
	struct YASL_Set *right = YASL_Set_new();
	YASL_Set_insert(right, YASL_INT(1));
	YASL_Set_insert(right, YASL_INT(6));
	YASL_Set_insert(right, YASL_INT(4));
	ASSERT_EQ(YASL_Set_length(right), 3);
	struct YASL_Set *set = YASL_Set_union(left, right);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(1))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(3))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(2))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(0))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(4))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(6))), true);
	ASSERT_EQ(YASL_Set_length(set), 5);
}

static void testintersectionset(void) {
	struct YASL_Set *left = YASL_Set_new();
	YASL_Set_insert(left, YASL_INT(1));
	YASL_Set_insert(left, YASL_INT(2));
	YASL_Set_insert(left, YASL_INT(3));
	ASSERT_EQ(YASL_Set_length(left), 3);
	struct YASL_Set *right = YASL_Set_new();
	YASL_Set_insert(right, YASL_INT(1));
	YASL_Set_insert(right, YASL_INT(6));
	YASL_Set_insert(right, YASL_INT(4));
	ASSERT_EQ(YASL_Set_length(right), 3);
	struct YASL_Set *set = YASL_Set_intersection(left, right);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(1))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(3))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(2))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(0))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(4))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(6))), false);
	ASSERT_EQ(YASL_Set_length(set), 1);
}

static void testsymmetricdifferenceset(void) {
	struct YASL_Set *left = YASL_Set_new();
	YASL_Set_insert(left, YASL_INT(1));
	YASL_Set_insert(left, YASL_INT(2));
	YASL_Set_insert(left, YASL_INT(3));
	ASSERT_EQ(YASL_Set_length(left), 3);
	struct YASL_Set *right = YASL_Set_new();
	YASL_Set_insert(right, YASL_INT(1));
	YASL_Set_insert(right, YASL_INT(6));
	YASL_Set_insert(right, YASL_INT(4));
	ASSERT_EQ(YASL_Set_length(right), 3);
	struct YASL_Set *set = YASL_Set_symmetric_difference(left, right);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(1))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(3))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(2))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(0))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(4))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(6))), true);
	ASSERT_EQ(YASL_Set_length(set), 4);
}

static void testdifferenceset(void) {
	struct YASL_Set *left = YASL_Set_new();
	YASL_Set_insert(left, YASL_INT(1));
	YASL_Set_insert(left, YASL_INT(2));
	YASL_Set_insert(left, YASL_INT(3));
	ASSERT_EQ(YASL_Set_length(left), 3);
	struct YASL_Set *right = YASL_Set_new();
	YASL_Set_insert(right, YASL_INT(1));
	YASL_Set_insert(right, YASL_INT(6));
	YASL_Set_insert(right, YASL_INT(4));
	ASSERT_EQ(YASL_Set_length(left), 3);
	struct YASL_Set *set = YASL_Set_difference(left, right);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(1))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(3))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(2))), true);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(0))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(4))), false);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(6))), false);
	ASSERT_EQ(YASL_Set_length(set), 2);
}

static void testremoveset(void) {
	struct YASL_Set *set = YASL_Set_new();
	YASL_Set_insert(set, YASL_INT(1));
	ASSERT_EQ(YASL_Set_length(set), 1);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(1))), true);
	YASL_Set_rm(set, YASL_INT(1));
	ASSERT_EQ(YASL_Set_length(set), 0);
	ASSERT_EQ((YASL_Set_search(set, YASL_INT(1))), false);
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
