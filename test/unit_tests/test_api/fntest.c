#include "yats.h"
#include "yasl.h"
#include "yasl_state.h"

SETUP_YATS();

static int TEST_FN(struct YASL_State *S) {
	yasl_int right = YASL_popint(S);
	yasl_int left = YASL_popint(S);

	YASL_pushint(S, left + right);
	return 1;
}

static void testfncall(void) {
	const char *code = "";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	//ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_compile(S));
	ASSERT_SUCCESS(YASL_execute(S));
	//ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	YASL_pushcfunction(S, TEST_FN, 2);
	YASL_pushint(S, 2);
	YASL_pushint(S, 5);
	int rets = YASL_functioncall(S, 2);
	ASSERT_EQ(rets, 1);
	ASSERT_EQ(YASL_peekint(S), 7);
	YASL_delstate(S);
}

TEST(fntest) {
	testfncall();
	return NUM_FAILED;
}
