#include "test/yats.h"
#include "interpreter/GC.h"

#include "data-structures/YASL_List.h"
#include "data-structures/YASL_Table.h"

SETUP_YATS();


static TEST(simple_alloc) {
	struct YASL_Object ptrs[10] = { YASL_END() };
	struct GC gc;
	gc_init(&gc);

	ptrs[0] = gc_alloc_list(&gc);

	ASSERT_EQ(gc_total_alloc_count(&gc), 1);

	ptrs[0] = YASL_END();

	gc_collect(&gc, ptrs, 10);

	ASSERT_EQ(gc_total_alloc_count(&gc), 0);

	gc_cleanup(&gc);
	return NUM_FAILED;
}

static TEST(multiple_allocs) {
	struct YASL_Object ptrs[10] = { YASL_END() };
	struct GC gc;
	gc_init(&gc);

	ptrs[0] = gc_alloc_list(&gc);

	ASSERT_EQ(gc_total_alloc_count(&gc), 1);

	ptrs[0] = gc_alloc_list(&gc);

	ASSERT_EQ(gc_total_alloc_count(&gc), 2);

	ptrs[1] = gc_alloc_list(&gc);

	ASSERT_EQ(gc_total_alloc_count(&gc), 3);

	gc_collect(&gc, ptrs, 10);

	ASSERT_EQ(gc_total_alloc_count(&gc), 2);

	ptrs[0] = YASL_END();
	ptrs[1] = YASL_END();

	gc_collect(&gc, ptrs, 10);

	ASSERT_EQ(gc_total_alloc_count(&gc), 0);

	gc_cleanup(&gc);
	return NUM_FAILED;
}

static TEST(simple_cycle) {
	struct YASL_Object ptrs[10] = { YASL_END() };
	struct GC gc;
	gc_init(&gc);

	ptrs[0] = gc_alloc_list(&gc);
	struct YASL_Object tmp = ptrs[0];

	YASL_List_push(YASL_GETLIST(tmp), tmp);

	ASSERT_EQ(gc_total_alloc_count(&gc), 1);

	gc_collect(&gc, ptrs, 10);

	ASSERT_EQ(gc_total_alloc_count(&gc), 1);

	ptrs[0] = YASL_END();

	ASSERT_EQ(gc_total_alloc_count(&gc), 1);

	gc_collect(&gc, ptrs, 10);

	ASSERT_EQ(gc_total_alloc_count(&gc), 0);

	gc_cleanup(&gc);
	return NUM_FAILED;
}

static TEST(simple_tree) {
	struct YASL_Object ptrs[10] = { YASL_END() };
	struct GC gc;
	gc_init(&gc);

	ptrs[0] = gc_alloc_list(&gc);

	for (int i = 0; i < 3; i++) {
		struct YASL_Object tmp = gc_alloc_list(&gc);
		YASL_List_push(YASL_GETLIST(ptrs[0]), tmp);
	}

	ASSERT_EQ(gc_total_alloc_count(&gc), 4);

	gc_collect(&gc, ptrs, 10);

	ASSERT_EQ(gc_total_alloc_count(&gc), 4);

	ptrs[0] = YASL_END();

	gc_collect(&gc, ptrs, 10);

	ASSERT_EQ(gc_total_alloc_count(&gc), 0);

	gc_cleanup(&gc);
	return NUM_FAILED;
}

int gctest(void) {
	RUN(simple_alloc);
	RUN(multiple_allocs);
	RUN(simple_cycle);
	RUN(simple_tree);

	return NUM_FAILED;
}