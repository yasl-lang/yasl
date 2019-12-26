#include "syntaxerrortest.h"

#include "yats.h"
#include "yasl_state.h"
#include "yasl.h"

SETUP_YATS();

#define ASSERT_SYNTAX_ERR(code, expected) {\
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));\
	S->compiler.parser.lex.err.print = io_print_string;\
	int result = YASL_compile(S);\
	ASSERT_EQ(result, 4);\
	const char *exp_err = "SyntaxError: " expected ".\n";\
	ASSERT_EQ(strlen(exp_err), S->compiler.parser.lex.err.len);\
	ASSERT_STR_EQ(exp_err, S->compiler.parser.lex.err.string, S->compiler.parser.lex.err.len);\
	YASL_delstate(S);\
}

int syntaxerrortest(void) {
	ASSERT_SYNTAX_ERR("for let x = 0; x < 5; x += 1 { };\necho x;", "Undeclared variable x (line 2)");
	ASSERT_SYNTAX_ERR("const x = 10; x = 11;", "Cannot assign to constant x (line 1)");
	ASSERT_SYNTAX_ERR("const x = 10; let x = 11;", "Illegal redeclaration of x (line 1)");
	ASSERT_SYNTAX_ERR("let x = 10; let x = 11;", "Illegal redeclaration of x (line 1)");
	ASSERT_SYNTAX_ERR("let x = [b for b <- [1, 2, 3, 4] if b % 2 == 0]; echo b;", "Undeclared variable b (line 1)");
	ASSERT_SYNTAX_ERR("echo if;", "Expected expression, got `if` (line 1)");
	ASSERT_SYNTAX_ERR("x;", "Undeclared variable x (line 1)");
	ASSERT_SYNTAX_ERR("echo 'hello \\o world'\n", "Invalid string escape sequence in line 1");
	ASSERT_SYNTAX_ERR("echo 'hello \\xworld'\n", "Invalid hex string escape in line 1");

	return __YASL_TESTS_FAILED__;
}
