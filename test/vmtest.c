#define RED
//#define RED "\x1B[31m"
#define END
//#define END "\n\x1B[0m"
#define EEND "." END
#define STR(x) #x
#define TST __FILE__, __LINE__

#define TYPEERR(s) RED "TypeError: " s EEND, 5, TST

#define METHOD(me, arg, xpc, act) TYPEERR(STR(me) " expected arg in position " STR(arg) " to be of type " STR(xpc) ", got arg of type " STR(act))
#define SYNTAX(s) RED "SyntaxError: " s EEND, 4, TST

#define METHOD_3_TEST(v, t, me, v0, v1, v2, pos, te, ta) \
  {STR(echo v.me(v0, v1, v2);), METHOD(t.me, pos, te, ta)}

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

		// Type errors (math)
	 METHOD_3_TEST(math, math, max, 1, .a, 2, 1, float, str),
	 METHOD_3_TEST(math, math, min, 1, .a, 2, 1, float, str)

	};

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
