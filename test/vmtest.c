#define RED "\x1B[31m"
#define END "\n\x1B[0m"
#define EEND "." END
#define STR(x) #x
#define TST __FILE__, __LINE__

#define TYPEERR(s) RED "TypeError: " s EEND, 5, TST

#define BINOP(op, l, r) TYPEERR(op " not supported for operands of types " STR(l) " and " STR(r))
#define UNOP(op, arg) TYPEERR(op " not supported for operand of type " STR(arg))
#define METHOD(me, arg, xpc, act) TYPEERR(STR(me) " expected arg in position " STR(arg) " to be of type " STR(xpc) ", got arg of type " STR(act))
#define SYNTAX(s) RED "SyntaxError: " s EEND, 4, TST
#define VALUE(s) RED "ValueError: " s EEND, 7, TST
#define DIVZRO RED "DivisionByZeroError" END, 6, TST

#define BINOP_TEST(op) {"echo .true " op " false;", BINOP(op, str, bool)}
#define UNOP_TEST(op) {"echo " op "true;", UNOP(op, bool)}
#define METHOD_1_TEST(v0, t0, me, v1, t1) \
  {STR(echo v0.me(v1);), METHOD(t0.me, 0, t0, t1)}
#define METHOD_2_TEST(v, t, me, v0, v1, pos, te, ta) \
  {STR(echo v.me(v0, v1);), METHOD(t.me, pos, te, ta)}
#define METHOD_2_TST(v, t, me, v0, v1, pos, ta)	\
  METHOD_2_TEST(v, t, me, v0, v1, pos, t, ta)
#define METHOD_TEST(v0, t0, me, v1, te, ta) \
  {STR(echo v0->me(v1);), METHOD(t0.me, 1, te, ta)}
#define METHOD_TST(v0, t0, me, v1, t1) METHOD_TEST(v0, t0, me, v1, t0, t1)
#define METHOD_3_TEST(v, t, me, v0, v1, v2, pos, te, ta) \
  {STR(echo v.me(v0, v1, v2);), METHOD(t.me, pos, te, ta)}
#define METHOD_3_TST(v, t, me, v0, v1, v2, pos, ta) \
  METHOD_3_TEST(v, t, me, v0, v1, v2, pos, t, ta)

static const struct {
	const char *toeval;
	const char *plaint;
  	int err;
	const char *file;
	int line;
} tests[] =
  // Syntax errors
  {{"for let x = 0; x < 5; x += 1 { };\necho x;",
    SYNTAX("Undeclared variable x (line 2)")},
   {"const x = 10; x = 11;",
    SYNTAX("Cannot assign to constant x (line 1)")},
   {"const x = 10; let x = 11;",
    SYNTAX("Illegal redeclaration of x (line 1)")},
   {"let x = 10; let x = 11;",
    SYNTAX("Illegal redeclaration of x (line 1)")},
   {"let x = [b for b <- [1, 2, 3, 4] if b % 2 == 0]; echo b;",
    SYNTAX("Undeclared variable b (line 1)")},
   {"echo if;",
    SYNTAX("ParsingError in line 1: expected expression, got `if`")},
   {"x;",
    SYNTAX("Undeclared variable x (line 1)")},
   {"echo 'hello \\o world'\n",
    SYNTAX("Invalid string escape sequence in line 1")},
   {"echo 'hello \\xworld'\n",
    SYNTAX("Invalid hex string escape in line 1")},

   // Type errors (operators)
   BINOP_TEST("|"),
   BINOP_TEST("^"),
   BINOP_TEST("&^"),
   BINOP_TEST("&"),
   BINOP_TEST(">>"),
   BINOP_TEST("<<"),
   BINOP_TEST("+"),
   BINOP_TEST("-"),
   BINOP_TEST("*"),
   BINOP_TEST("/"),
   BINOP_TEST("//"),
   BINOP_TEST("%"),
   BINOP_TEST("**"),
   UNOP_TEST("-"),
   UNOP_TEST("+"),
   {"echo len true;", UNOP("len", bool)},
   UNOP_TEST("^"),

   // Type errors (bool methods)
   METHOD_1_TEST(true, bool, tostr, 0, int),
   METHOD_1_TEST(true, bool, tobool, 0, int),

   // Type errors (float methods)
   METHOD_1_TEST(0.0, float, tostr, 1, int),
   METHOD_1_TEST(0.0, float, toint, 1, int),
   METHOD_1_TEST(0.0, float, tofloat, 1, int),
   METHOD_1_TEST(0.0, float, tobool, 0, int),

   // Type errors (int methods)
   METHOD_1_TEST(0, int, tostr, 1.0, float),
   METHOD_1_TEST(0, int, tofloat, 1.0, float),
   METHOD_1_TEST(0, int, toint, 1.0, float),
   METHOD_1_TEST(0, int, tobool, 1.0, float),

   // Type errors (list methods)
   METHOD_2_TST([], list, push, 1, true, 0, int),
   METHOD_1_TEST([], list, copy, true, bool),
   METHOD_2_TST([], list, __add, true, [], 0, bool),
   METHOD_2_TST([], list, __add, true, 1, 1, int),
   METHOD_2_TST([], list, __add, [], true, 1, bool),
   {"echo [] + true;",
    METHOD(list.__add, 1, list, bool)},
   METHOD_TST([], list, __add, true, bool),
   METHOD_2_TST([], list, extend, 1, [], 0, int),
   METHOD_2_TST([], list, extend, 1, 1.0, 1, float),
   METHOD_2_TST([], list, extend, [], true, 1, bool),
   METHOD_TST([], list, extend, 1, int),
   // TODO: __get, __set
   METHOD_1_TEST([], list, tostr, 1, int),
   METHOD_2_TST([], list, search, 1, .str, 0, int),
   METHOD_1_TEST([], list, reverse, 1, int),
   METHOD_1_TEST([], list, clear, 1, int),
   METHOD_2_TST([], list, join, 1, .str, 0, int),
   METHOD_2_TEST([], list, join, 1, true, 1, str, bool),
   METHOD_2_TEST([], list, join, [], 1, 1, str, int),
   METHOD_TEST([], list, join, 1, str, int),
   METHOD_1_TEST([], list, sort, 1, int),

   // Type errors (string methods)
   METHOD_1_TEST('', str, tofloat, 1, int),
   METHOD_1_TEST('', str, toint, 1, int),
   METHOD_1_TEST('', str, isalnum, 1, int),
   METHOD_1_TEST('', str, isal, 1, int),
   METHOD_1_TEST('', str, isnum, 1, int),
   METHOD_1_TEST('', str, isspace, 1, int),
   METHOD_1_TEST('', str, tobool, 1, int),
   METHOD_1_TEST('', str, tostr, 1, int),
   METHOD_1_TEST('', str, toupper, 1, int),
   METHOD_1_TEST('', str, tolower, 1, int),

   METHOD_2_TST('', str, startswith, 1, true, 1, bool),
   METHOD_2_TST('', str, startswith, 1, .true, 0, int),
   METHOD_2_TST('', str, startswith, .str, true, 1, bool),
   METHOD_TST('', str, startswith, 1, int),

   METHOD_2_TST('', str, endswith, 1, true, 1, bool),
   METHOD_2_TST('', str, endswith, 1, .true, 0, int),
   METHOD_2_TST('', str, endswith, .str, true, 1, bool),
   METHOD_TST('', str, endswith, 1, int),

   METHOD_3_TST('', str, replace, 1, true, 1.0, 2, float),
   METHOD_3_TST('', str, replace, 1, true, .str, 1, bool),
   METHOD_3_TST('', str, replace, 1, .true, .str, 0, int),
   METHOD_3_TST('', str, replace, 1, .true, true, 2, bool),
   METHOD_3_TST('', str, replace, .tr, true, .str, 1, bool),
   METHOD_3_TST('', str, replace, .tr, .true, 1, 2, int),
   METHOD_3_TST('', str, replace, .tr, true, 1, 2, int),

   METHOD_2_TST('', str, search, 1, true, 1, bool),
   METHOD_2_TST('', str, search, 1, .true, 0, int),
   METHOD_2_TST('', str, search, .str, true, 1, bool),
   METHOD_TST('', str, search, 1, int),

   METHOD_2_TST('', str, count, 1, true, 1, bool),
   METHOD_2_TST('', str, count, 1, .true, 0, int),
   METHOD_2_TST('', str, count, .str, true, 1, bool),
   METHOD_TST('', str, count, 1, int),

   METHOD_2_TST('', str, split, 1, true, 1, bool),
   METHOD_2_TST('', str, split, 1, .true, 0, int),
   METHOD_2_TST('', str, split, .str, true, 1, bool),
   METHOD_TST('', str, split, 1, int),

   METHOD_2_TST('', str, ltrim, 1, true, 1, bool),
   METHOD_2_TST('', str, ltrim, 1, .true, 0, int),
   METHOD_2_TST('', str, ltrim, .str, true, 1, bool),
   METHOD_TST('', str, ltrim, 1, int),

   METHOD_2_TST('', str, rtrim, 1, true, 1, bool),
   METHOD_2_TST('', str, rtrim, 1, .true, 0, int),
   METHOD_2_TST('', str, rtrim, .str, true, 1, bool),
   METHOD_TST('', str, rtrim, 1, int),

   METHOD_2_TST('', str, trim, 1, true, 1, bool),
   METHOD_2_TST('', str, trim, 1, .true, 0, int),
   METHOD_2_TST('', str, trim, .str, true, 1, bool),
   METHOD_TST('', str, trim, 1, int),

   METHOD_2_TEST('', str, rep, 1, true, 1, int, bool),
   METHOD_2_TST('', str, rep, 1.0, 1, 0, float),
   METHOD_2_TEST('', str, rep, .str, true, 1, int, bool),
   METHOD_TEST('', str, rep, true, int, bool),

   // Type errors (table methods)
   METHOD_2_TST({}, table, remove, 1, 2.0, 0, int),
   METHOD_1_TEST({}, table, keys, 1, int),
   METHOD_1_TEST({}, table, values, 1, int),
   METHOD_1_TEST({}, table, copy, 1, int),
   METHOD_1_TEST({}, table, tostr, 1, int),
   // TODO: __get, __set
   METHOD_1_TEST({}, table, clear, 1, int),

   // Type errors (undef methods)
   METHOD_1_TEST(undef, undef, tostr, 1, int),

   // Type errors (math)
   METHOD_3_TEST(math, math, max, 1, .a, 2, 1, float, str),
   METHOD_3_TEST(math, math, min, 1, .a, 2, 1, float, str),

   // Value errors
   {"echo []->pop();",
    VALUE("list.pop expected nonempty list as arg 0")},
   {"echo [1, .a]->sort();",
    VALUE("list.sort expected a list of all numbers or all strings")},
   {"echo ''.replace(.tr, '', .sad);",
    VALUE("str.replace expected a nonempty str as arg 1")},
   {"echo 'wasd'->split('');",
    VALUE("str.split expected a nonempty str as arg 1")},
   {"echo 'as'->rep(-1);",
    VALUE("str.rep expected non-negative int as arg 1")},

   // Division by zero errors
   {"echo 1 // 0;", DIVZRO},
   {"echo 1 % 0;", DIVZRO}};

// This is here to avoid messing up true/false/bool.
#include "test/test_util.h"
#include <string.h>
#include <stdio.h>
#define PREF vm

int MAIN(void) {
	int fails = 0;
	const char *args[] = {"-E", "", NULL};
	for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		args[1] = tests[i].toeval;
		if (assert_output("./yasl", args,
				  0, "",
				  strlen(tests[i].plaint),
				  tests[i].plaint, tests[i].err,
				  tests[i].file, tests[i].line)) {
			fails++;
		}
	}
	REPORT((int)(sizeof(tests)/sizeof(tests[0])), fails);
	return !!fails;
}
