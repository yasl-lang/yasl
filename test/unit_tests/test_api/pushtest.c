#include "yats.h"
#include "yasl.h"
#include "yasl_state.h"

SETUP_YATS();

static void testrun(void) {
	struct YASL_State *S = YASL_newstate_bb("", 0);
	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

static void testsetglobal_undef(void) {
	struct YASL_State *S = YASL_newstate_bb("echo x;", strlen("echo x;"));
	S->vm.out.print = io_print_string;
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	YASL_pushundef(S);
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));
	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

static void testsetglobal_float(void) {
	struct YASL_State *S = YASL_newstate_bb("echo x;", strlen("echo x;"));
	S->vm.out.print = io_print_string;
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	YASL_pushfloat(S, 12.5);
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));
	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

static void testsetglobal_bool(void) {
	struct YASL_State *S = YASL_newstate_bb("echo x;", strlen("echo x;"));
	S->vm.out.print = io_print_string;
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	YASL_pushbool(S, true);
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));
	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

static void testsetglobal_int(void) {
	struct YASL_State *S = YASL_newstate_bb("echo x;", strlen("echo x;"));
	S->vm.out.print = io_print_string;
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	YASL_pushint(S, 12);
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));
	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

static void testsetglobal_szstr(void) {
	struct YASL_State *S = YASL_newstate_bb("echo x;", strlen("echo x;"));
	char *tmp = (char *)malloc(strlen("hello world") + 1);
	strcpy(tmp, "hello world");
	S->vm.out.print = io_print_string;
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	YASL_pushzstr(S, tmp);
	free(tmp);
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));
	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

static void testsetglobal_litstr(void) {
	struct YASL_State *S = YASL_newstate_bb("echo x;", strlen("echo x;"));
	S->vm.out.print = io_print_string;
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	YASL_pushlit(S, "hello world");
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));
	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

static void testsetglobal_list(void) {
	struct YASL_State *S = YASL_newstate_bb("echo x;", strlen("echo x;"));
	S->vm.out.print = io_print_string;
	ASSERT_SUCCESS(YASL_declglobal(S, "x"));
	YASL_pushlist(S);
	ASSERT_SUCCESS(YASL_setglobal(S, "x"));
	ASSERT_SUCCESS(YASL_execute(S));
	YASL_delstate(S);
}

TEST(pushtest) {
	testrun();
	testsetglobal_undef();
	testsetglobal_float();
	testsetglobal_bool();
	testsetglobal_int();
	testsetglobal_szstr();
	testsetglobal_litstr();
	testsetglobal_list();
	return NUM_FAILED;
}
