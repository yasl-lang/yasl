#include "yats.h"
#include "yasl.h"

SETUP_YATS();

static void testpopfloat(void) {
	char *code = "x = 12.5;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_compile(S));
	ASSERT_SUCCESS(YASL_execute(S));
	struct YASL_Object *X = YASL_getglobal(S, "x");
	ASSERT_EQ(X != NULL, true);
	yasl_float x = YASL_getdouble(X);
	ASSERT_EQ(x, 12.5);
	YASL_delstate(S);
}

static void testpopint(void) {
	char *code = "x = 12;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_compile(S));
	ASSERT_SUCCESS(YASL_execute(S));
	struct YASL_Object *X = YASL_getglobal(S, "x");
	ASSERT_EQ(X != NULL, true);
	yasl_int x = YASL_getinteger(X);
	ASSERT_EQ(x, 12);
	YASL_delstate(S);
}

static void testpopbool(void) {
	char *code = "x = true;";
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	ASSERT_SUCCESS(YASL_compile(S));
	ASSERT_SUCCESS(YASL_execute(S));
	struct YASL_Object *X = YASL_getglobal(S, "x");
	ASSERT_EQ(X != NULL, true);
	bool x = YASL_getboolean(X);
	ASSERT_EQ(x, true);
	YASL_delstate(S);
}

int poptest(void) {
	testpopfloat();
	testpopint();
	testpopbool();
	return __YASL_TESTS_FAILED__;
}
