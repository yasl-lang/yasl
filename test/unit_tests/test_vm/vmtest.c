#include "test/yats.h"
#include "yasl.h"
#include "yasl_aux.h"
#include "IO.h"
#include "yasl_state.h"

SETUP_YATS();

#define STR(x) #x

#define ASSERT_VALUE_ERR(code, expected, line) {\
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));\
	YASLX_decllibs(S);\
	S->vm.err.print = io_print_string;\
	S->vm.out.print = io_print_string;\
	ASSERT_SUCCESS(YASL_compile(S));\
	int result = YASL_execute(S);\
	ASSERT_EQ(result, 7);\
	const char *exp_err = "ValueError: " expected ". (line " STR(line) ")\n";\
	ASSERT_EQ(S->vm.out.len, 0);\
	ASSERT_EQ(strlen(exp_err), S->vm.err.len);\
	ASSERT_STR_EQ(exp_err, S->vm.err.string, S->vm.err.len);\
	YASL_delstate(S);\
}

////////////////////////////////////////////////////////////////////////////////

int vmtest(void) {
	// io errors
	ASSERT_VALUE_ERR("let f = io.open('f', 'www');", "io.open was passed invalid mode: www", 1);
	ASSERT_VALUE_ERR("let f = io.open('f', 'y+');", "io.open was passed invalid mode: y+", 1);
	ASSERT_VALUE_ERR("let f = io.open('f', 'p');", "io.open was passed invalid mode: p", 1);
	ASSERT_VALUE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f->read('y');",
			    "io.file.read was passed invalid mode: y", 1);
	ASSERT_VALUE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f->read('ysadasd');",
			 "io.file.read was passed invalid mode: ysadasd", 1);

	// ASSERT_TYPE_ERR("mt.setmt(1, {});", "cannot set metatable for value of type int", 1);
	// ASSERT_ARG_TYPE_ERR("mt.setmt({}, 1);", "mt.setmt", "table", "int", 1, 1);
	return __YASL_TESTS_FAILED__;
}
