#include "yats.h"
#include "yasl.h"
#include "yasl_state.h"

SETUP_YATS();

static void testtablenext(void) {
	const char *code = "x = { 1: 1.0, 2: 2.0, 3: 3.0 };";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	YASL_declglobal(S, "x");
	ASSERT_SUCCESS(YASL_execute(S));
	ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	ASSERT(YASL_istable(S));

	int i = 0;
	YASL_pushundef(S);
	while (YASL_tablenext(S)) {
		ASSERT(YASL_isnfloat(S, 2));
		YASL_pop(S);
		ASSERT(YASL_isnint(S, 1));
		i++;
	}

	ASSERT(YASL_istable(S));
	ASSERT_EQ(i, 3);

	YASL_delstate(S);
}

TEST(tablenexttest) {
	testtablenext();
	return NUM_FAILED;
}
