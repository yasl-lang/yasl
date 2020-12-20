#include "listtest.h"
#include "interpreter/YASL_Object.h"
#include "test/yats.h"
#include "data-structures/YASL_List.h"

SETUP_YATS();

/// check that list_append is resizing the list properly.
/// Does not check that correct elements are appended
static void testlistresizing(void) {
	struct YASL_List *list = YASL_List_new_sized(0);
	YASL_List_append(list, YASL_INT(0));
	ASSERT_EQ(list->size, 1);
	ASSERT_EQ(list->count, 1);
	YASL_List_append(list, YASL_INT(5));
	ASSERT_EQ(list->size, 2);
	ASSERT_EQ(list->count, 2);
	YASL_List_append(list, YASL_INT(10));
	ASSERT_EQ(list->size, 4);
	ASSERT_EQ(list->count, 3);
	YASL_List_append(list, YASL_INT(20));
	ASSERT_EQ(list->size, 4);
	ASSERT_EQ(list->count, 4);
	YASL_List_append(list, YASL_INT(40));
	ASSERT_EQ(list->size, 8);
	ASSERT_EQ(list->count, 5);
}



TEST(listtest) {
	testlistresizing();
	return NUM_FAILED;
}
