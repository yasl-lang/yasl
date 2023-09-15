#include "yats.h"
#include "yasl.h"
#include "yasl_state.h"

SETUP_YATS();

static void testpopfloat(void) {
	const char *code = "x = 12.5;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	YASL_declglobal(S, "x");
	ASSERT_SUCCESS(YASL_execute(S));
	ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	ASSERT(YASL_isnfloat(S, 0));
	ASSERT_EQ(YASL_peeknfloat(S, 0), 12.5);
	YASL_delstate(S);
}

static void testpopint(void) {
	const char *code = "x = 12;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	YASL_declglobal(S, "x");
	ASSERT_SUCCESS(YASL_execute(S));
	ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	ASSERT(YASL_isnint(S, 0));
	ASSERT_EQ(YASL_peeknint(S, 0), 12);
	YASL_delstate(S);
}

static void testpopbool(void) {
	const char *code = "x = true;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	YASL_declglobal(S, "x");
	ASSERT_SUCCESS(YASL_execute(S));
	ASSERT_SUCCESS(YASL_loadglobal(S, "x"));
	ASSERT(YASL_isnbool(S, 0));
	ASSERT_EQ(YASL_peeknbool(S, 0), true);
	YASL_delstate(S);
}

static int testpop_userptr_helper(struct YASL_State *S) {
	char *x = (char *)YASL_popuserptr(S);
	YASL_pushlit(S, x);
	return 1;
}

static void testpopuserptr(void) {
	const char *code = "const tmp = f(x);";
	char x[] = "hello world";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));

	YASL_declglobal(S, "x");
	YASL_pushuserptr(S, x);
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));

	YASL_declglobal(S, "f");
	YASL_pushcfunction(S, testpop_userptr_helper, 1);
	ASSERT_SUCCESS(YASL_setglobal(S, "f"));

	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

TEST(poptest) {
	testpopfloat();
	testpopint();
	testpopbool();
	testpopuserptr();
	return NUM_FAILED;
}
