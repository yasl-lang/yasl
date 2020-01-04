#include "test/test_util.h"
#include <string.h>
#include <stdio.h>
#define PREF cli

const char *const cmd = "./yasl";

#define USAGESTR "usage: yasl [option] [input]\n"\
  "options:\n"\
  "\t-C: checks `input` for syntax errors but doesn't run it.\n"\
  "\t-e input: executes `input` as code and prints result of last statement.\n"\
  "\t-E input: executes `input` as code.\n"\
  "\t-h: show this text.\n"\
  "\t-V: print current version.\n"\
  "\tinput: name of file containing script (or literal to execute with -e or -E).\n"

#define TST __FILE__, __LINE__

static const struct {
	const char *out;
	const char *file;
	int line;
	const char *args[3];
} tests[] =
	{{"YASL v0.6.6\n", TST, {"-V", NULL}},
	 {"10\n", TST, {"-e", "let x=10; x;", NULL}},
	 {"", TST, {"-E", "let x=10; x;", NULL}},
	 {"10\n", TST, {"-E", "let x=10; echo x;", NULL}},
	 {USAGESTR, TST, {"-h", NULL}}};


int MAIN(void) {
	int fails = 0;
	for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		if (assert_output(cmd, tests[i].args,
				  strlen(tests[i].out), tests[i].out,
				  0, NULL, 0,
				  tests[i].file, tests[i].line)) {
			fails++;
		}
	}
	REPORT((int)(sizeof(tests)/sizeof(tests[0])), fails);
	return !!fails;
}
