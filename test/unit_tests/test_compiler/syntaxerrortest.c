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

TEST(syntaxerrortest) {
	ASSERT_SYNTAX_ERR("for let x = 0; x < 5; x += 1 { };\necho x;", "Undeclared variable x (line 2)");

	ASSERT_SYNTAX_ERR("const x = 10; x = 11;", "Cannot assign to constant x (line 1)");
	ASSERT_SYNTAX_ERR("const x = 10; let x = 11;", "Illegal redeclaration of x (line 1)");
	ASSERT_SYNTAX_ERR("let x = 10; let x = 11;", "Illegal redeclaration of x (line 1)");
	ASSERT_SYNTAX_ERR("let x = 10; const x = 11;", "Illegal redeclaration of x (line 1)");
	ASSERT_SYNTAX_ERR("const x = 10; const x = 11;", "Illegal redeclaration of x (line 1)");
	ASSERT_SYNTAX_ERR("let x = [b for b <- [1, 2, 3, 4] if b % 2 == 0]; echo b;", "Undeclared variable b (line 1)");
	ASSERT_SYNTAX_ERR("echo if;", "Expected expression, got `if` (line 1)");
	ASSERT_SYNTAX_ERR("else { echo true };", "`else` without previous `if` (line 1)");
	ASSERT_SYNTAX_ERR("elseif { echo true };", "`elseif` without previous `if` (line 1)");
	ASSERT_SYNTAX_ERR("5 = 10;", "Invalid l-value (line 1)");
	ASSERT_SYNTAX_ERR("5 += 10;", "Invalid l-value (line 1)");
	ASSERT_SYNTAX_ERR("x->5();", "Invalid method call (line 1)");
	ASSERT_SYNTAX_ERR("x.5;", "Invalid member access (line 1)");
	ASSERT_SYNTAX_ERR("fn outer() {\n"
			  "  const a = 10\n"
			  "  let b = 12\n"
			  "  fn middle() {\n"
			  "    fn inner() {\n"
			  "      echo a;\n"
			  "      a = 100;\n"
			  "    }\n"
			  "    return inner\n"
			  "  }\n"
			  "  middle()()\n"
			  "  return middle()\n"
			  "}\n"
			  "\n"
			  "let inside = outer()\n"
			  "inside()\n", "Cannot assign to constant a (line 7)");
	ASSERT_SYNTAX_ERR("let f = fn(n) { return n == 0 ? 1 : f(n-1); };", "Undeclared variable f (line 1)");
	ASSERT_SYNTAX_ERR("x;", "Undeclared variable x (line 1)");
	ASSERT_SYNTAX_ERR("let x = x;", "Undeclared variable x (line 1)");
	ASSERT_SYNTAX_ERR("echo 'hello \\o world'\n", "Invalid string escape sequence in line 1");
	ASSERT_SYNTAX_ERR("echo 'hello \\xworld'\n", "Invalid hex string escape in line 1");
	ASSERT_SYNTAX_ERR("0b__2", "Invalid int literal in line 1");

	ASSERT_SYNTAX_ERR("match 1 {\n"
			  "let x | const x {\n"
			  "}\n }\n", "x must be bound with either `const` or `let` on both sides of | (line 2)");
	ASSERT_SYNTAX_ERR("match 1 {\n"
			  "let x | {} {\n"
			  "}\n }\n", "x not bound on right side of | (line 2)");
	ASSERT_SYNTAX_ERR("match 1 {\n"
			  "{} | let y {\n"
			  "}\n }\n", "y not bound on left side of | (line 2)");
	ASSERT_SYNTAX_ERR("match 1 {\n"
			  "let x | let y {\n"
			  "}\n }\n", "y not bound on left side of | (line 2)");
	ASSERT_SYNTAX_ERR("match 1 {\n"
			  "[ let x, let x ] {\n"
			  "}\n }\n", "Illegal rebinding of x (line 2)");
	ASSERT_SYNTAX_ERR("match n {\n"
			  "    \"x: #{let x}\" {\n"
			  "    }\n"
			  "}\n", "Interpolated strings cannot be used in patterns (line 2)")


	ASSERT_SYNTAX_ERR("\"asdsadasd\n", "Unclosed string literal in line 1");
	ASSERT_SYNTAX_ERR("\"asdsadasd", "Unclosed string literal in line 1");
	ASSERT_SYNTAX_ERR("\"das#{1}asd\n", "Unclosed string literal in line 1");
	ASSERT_SYNTAX_ERR("\"das#{1}asd", "Unclosed string literal in line 1");
	ASSERT_SYNTAX_ERR("\"asdasd#{1 asdda\";", "Expected } in line 1");
	ASSERT_SYNTAX_ERR("'asdsadasd\n", "Unclosed string literal in line 1");
	ASSERT_SYNTAX_ERR("'asdsadasd", "Unclosed string literal in line 1");
	ASSERT_SYNTAX_ERR("`asdsadasd", "Unclosed string literal in line 1");
	ASSERT_SYNTAX_ERR("enum", "`enum` is an unused reserved word and cannot be used (line 1)");
	ASSERT_SYNTAX_ERR("do", "`do` is an unused reserved word and cannot be used (line 1)");
	ASSERT_SYNTAX_ERR("use", "`use` is an unused reserved word and cannot be used (line 1)");
	ASSERT_SYNTAX_ERR("no", "`no` is an unused reserved word and cannot be used (line 1)");
	ASSERT_SYNTAX_ERR("extern", "`extern` is an unused reserved word and cannot be used (line 1)");
	ASSERT_SYNTAX_ERR("/*..a.sd", "Unclosed block comment in line 1");
	ASSERT_SYNTAX_ERR("@", "Unknown character in line 1: `@` (0x40)");
	ASSERT_SYNTAX_ERR("if (true) { export 1; };", "`export` statement must be at top level of module (line 1)");
	ASSERT_SYNTAX_ERR("continue;", "`continue` outside of loop (line 1)");
	ASSERT_SYNTAX_ERR("break;", "`break` outside of loop (line 1)");
	ASSERT_SYNTAX_ERR("return 10;", "`return` outside of function (line 1)");
	ASSERT_SYNTAX_ERR("return 10, 11;", "`return` outside of function (line 1)");

	ASSERT_SYNTAX_ERR("echo if;", "Expected expression, got `if` (line 1)");
	ASSERT_SYNTAX_ERR("assert false", "Expected ;, got END OF FILE (line 1)");
	ASSERT_SYNTAX_ERR("echo ''[];", "Invalid expression `]` (line 1)");

	return NUM_FAILED;
}
