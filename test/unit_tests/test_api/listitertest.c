#include "yats.h"
#include "yasl.h"
#include "yasl_state.h"

SETUP_YATS();

static void testlistiter(void) {
	const char *code = "x = [ 10, 9, 8, 7, 6 ];";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_compile(S));
	ASSERT_SUCCESS(YASL_execute(S));
	ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	ASSERT(YASL_islist(S));

	YASL_duptop(S);
	YASL_len(S);
	yasl_int len = YASL_popint(S);
	ASSERT_EQ(len, 5);

	for (yasl_int i = 0; i < len; i++) {
		YASL_listget(S, i);
		ASSERT(YASL_isint(S));
		yasl_int value = YASL_popint(S);
		ASSERT_EQ(value, 10 - i);
	}

	YASL_delstate(S);
}

TEST(listitertest) {
	testlistiter();
	return NUM_FAILED;
}
