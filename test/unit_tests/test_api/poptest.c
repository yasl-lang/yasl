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

static int testpop_userptr_helper(struct YASL_State *S) {
	char *x = (char *)YASL_popuserptr(S);
	YASL_pushlitszstring(S, x);
	return YASL_SUCCESS;
}

static void testpopuserptr(void) {
	const char *code = "const tmp = f(x);";
	char x[] = "hello world";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));

	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_pushuserptr(S, x));
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));

	ASSERT_SUCCESS(YASL_declglobal(S, "f"));
	ASSERT_SUCCESS(YASL_pushcfunction(S, testpop_userptr_helper, 1));
	ASSERT_SUCCESS(YASL_setglobal(S, "f"));

	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

int poptest(void) {
	testpopfloat();
	testpopint();
	testpopbool();
	testpopuserptr();
	return __YASL_TESTS_FAILED__;
}
