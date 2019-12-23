#include "test/yats.h"
#include "yasl.h"
#include "IO.h"
#include "yasl_state.h"

SETUP_YATS();

#define STR(x) #x

#define ASSERT_VALUE_ERR(code, expected) {\
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));\
	S->vm.err.print = io_print_string;\
	S->vm.out.print = io_print_string;\
	ASSERT_SUCCESS(YASL_compile(S));\
	int result = YASL_execute(S);\
	ASSERT_EQ(result, 7);\
	const char *exp_err = "ValueError: " expected ".\n";\
	ASSERT_EQ(S->vm.out.len, 0);\
	ASSERT_EQ(strlen(exp_err), S->vm.err.len);\
	ASSERT_EQ(memcmp(exp_err, S->vm.err.string, S->vm.err.len), 0);\
	YASL_delstate(S);\
}

#define ASSERT_DIV_BY_ZERO_ERR(code) {\
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));\
	S->vm.err.print = io_print_string;\
	S->vm.out.print = io_print_string;\
	ASSERT_SUCCESS(YASL_compile(S));\
	int result = YASL_execute(S);\
	ASSERT_EQ(result, 6);\
	const char *exp_err = "DivisionByZeroError\n";\
	ASSERT_EQ(S->vm.out.len, 0);\
	ASSERT_EQ(strlen(exp_err), S->vm.err.len);\
	ASSERT_EQ(memcmp(exp_err, S->vm.err.string, S->vm.err.len), 0);\
	YASL_delstate(S);\
}

#define ASSERT_ARG_TYPE_ERR(code, method, exp, actual, arg) {\
	struct YASL_State *S = YASL_newstate_bb(code "\n", strlen(code));\
	S->vm.err.print = io_print_string;\
	S->vm.out.print = io_print_string;\
	ASSERT_SUCCESS(YASL_compile(S));\
	int result = YASL_execute(S);\
	ASSERT_EQ(result, 5);\
	const char *exp_err = \
		"TypeError: " method " expected arg in position " STR(arg) \
		" to be of type " exp ", got arg of type " actual ".\n";\
	ASSERT_EQ(S->vm.out.len, 0);\
	ASSERT_EQ(strlen(exp_err), S->vm.err.len);\
	if (memcmp(exp_err, S->vm.err.string, S->vm.err.len) != 0) {\
		printf(K_RED "%s =/= %s\n" K_END, S->vm.err.string, exp_err);\
	}\
	ASSERT_EQ(memcmp(exp_err, S->vm.err.string, S->vm.err.len), 0);\
	YASL_delstate(S);\
}

////////////////////////////////////////////////////////////////////////////////

int vmtest(void) {
	// bool method type errors
	ASSERT_ARG_TYPE_ERR("true.tostr(1);", "bool.tostr", "bool", "int", 0);
	ASSERT_ARG_TYPE_ERR("true.tobool(1);", "bool.tobool", "bool", "int", 0);

	// float method type errors
	ASSERT_ARG_TYPE_ERR("0.0.tostr(1);", "float.tostr", "float", "int", 0);
	ASSERT_ARG_TYPE_ERR("0.0.toint(1);", "float.toint", "float", "int", 0);
	ASSERT_ARG_TYPE_ERR("0.0.tofloat(1);", "float.tofloat", "float", "int", 0);
	ASSERT_ARG_TYPE_ERR("0.0.tobool(1);", "float.tobool", "float", "int", 0);

	// int method type errors
	ASSERT_ARG_TYPE_ERR("0.tostr(1.0);", "int.tostr", "int", "float", 0);
	ASSERT_ARG_TYPE_ERR("0.toint(1.0);", "int.toint", "int", "float", 0);
	ASSERT_ARG_TYPE_ERR("0.tofloat(1.0);", "int.tofloat", "int", "float", 0);
	ASSERT_ARG_TYPE_ERR("0.tobool(1.0);", "int.tobool", "int", "float", 0);

	// list method type errors
	ASSERT_ARG_TYPE_ERR("[].push(1, true);", "list.push", "list", "int", 0);
	ASSERT_ARG_TYPE_ERR("[].copy(true);", "list.copy", "list", "bool", 0);
	ASSERT_ARG_TYPE_ERR("[].__add(true, []);", "list.__add", "list", "bool", 0);
	ASSERT_ARG_TYPE_ERR("[].__add([], true);", "list.__add", "list", "bool", 1);
	ASSERT_ARG_TYPE_ERR("[].__add(true, 1);", "list.__add", "list", "int", 1);
	ASSERT_ARG_TYPE_ERR("[] + true;", "list.__add", "list", "bool", 1);
	// ASSERT_ARG_TYPE_ERR("true + [];", "list.__add", "list", "bool", 0);
	ASSERT_ARG_TYPE_ERR("[].extend(1, []);", "list.extend", "list", "int", 0);
	ASSERT_ARG_TYPE_ERR("[].extend([], 1);", "list.extend", "list", "int", 1);
	ASSERT_ARG_TYPE_ERR("[].extend(1, true);", "list.extend", "list", "bool", 1);
	// TODO: __get, __set
	ASSERT_ARG_TYPE_ERR("[].tostr(1);", "list.tostr", "list", "int", 0);
	ASSERT_ARG_TYPE_ERR("[].search(1);", "list.search", "list", "int", 0);
	ASSERT_ARG_TYPE_ERR("[].reverse(1);", "list.reverse", "list", "int", 0);
	ASSERT_ARG_TYPE_ERR("[].clear(1);", "list.clear", "list", "int", 0);
	ASSERT_ARG_TYPE_ERR("[].join(1, .str);", "list.join", "list", "int", 0);
	ASSERT_ARG_TYPE_ERR("[].join(1, true);", "list.join", "str", "bool", 1);
	ASSERT_ARG_TYPE_ERR("[].join([], true);", "list.join", "str", "bool", 1);
	ASSERT_ARG_TYPE_ERR("[].sort(1);", "list.sort", "list", "int", 0);

	// value errors
	ASSERT_VALUE_ERR("echo []->pop();", "list.pop expected nonempty list as arg 0");
	ASSERT_VALUE_ERR("echo [1, .a]->sort();", "list.sort expected a list of all numbers or all strings");
	ASSERT_VALUE_ERR("echo ''.replace(.tr, '', .sad);", "str.replace expected a nonempty str as arg 1");
	ASSERT_VALUE_ERR("echo 'wasd'->split('');", "str.split expected a nonempty str as arg 1");
	ASSERT_VALUE_ERR("echo 'as'->rep(-1);", "str.rep expected non-negative int as arg 1");

	// division by zero errors
	ASSERT_DIV_BY_ZERO_ERR("echo 1 // 0;");
	ASSERT_DIV_BY_ZERO_ERR("echo 1 % 0;");
	return __YASL_TESTS_FAILED__;
}
