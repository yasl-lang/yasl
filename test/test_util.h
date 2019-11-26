#pragma once
#include <stdbool.h>
#include <stddef.h>

extern bool assert_output(const char *prog, const char *const *args,
			  size_t eolen, const char *expected_output,
			  size_t eelen, const char *expected_error,
			  int expected_status,
			  const char *file, int line);
#define MAIN YMAIN(PREF)
#define YMAIN(x) XMAIN(x)
#define XMAIN(x) x ## _main
#define REPORT(tests, fails) printf("Ran %d tests. (%d failed.)\n", tests, fails)
