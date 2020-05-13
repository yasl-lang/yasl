#include "yats.h"
#include "yasl.h"
#include "yasl_state.h"

SETUP_YATS();

static void testpopfloat(void) {
	const char *code = "x = 12.5;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_compile(S));
	ASSERT_SUCCESS(YASL_execute(S));
	ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	ASSERT(YASL_isfloat(S));
	ASSERT_EQ(YASL_peekfloat(S), 12.5);
	YASL_delstate(S);
}

static void testpopint(void) {
	const char *code = "x = 12;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_compile(S));
	ASSERT_SUCCESS(YASL_execute(S));
	ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	ASSERT(YASL_isint(S));
	ASSERT_EQ(YASL_peekint(S), 12);
	YASL_delstate(S);
}

static void testpopbool(void) {
	const char *code = "x = true;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_compile(S));
	ASSERT_SUCCESS(YASL_execute(S));
	ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	ASSERT(YASL_isbool(S));
	ASSERT_EQ(YASL_peekbool(S), true);
	YASL_delstate(S);
}

int poptest(void) {
	testpopfloat();
	testpopint();
	testpopbool();
	return __YASL_TESTS_FAILED__;
}
