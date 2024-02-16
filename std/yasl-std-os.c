#include "yasl-std-os.h"

#include "yasl_aux.h"

#include <time.h>

static int YASL_os_getenv(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		YASL_print_err(S, "TypeError: expected str, got %s.", YASL_peekntypename(S, 0));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	char *name = YASL_popcstr(S);
	const char *value = getenv(name);
	free(name);

	if (value) {
		YASL_pushzstr(S, value);
	} else {
		YASL_pushundef(S);
	}

	return 1;
}

static int YASL_os_clock(struct YASL_State *S) {
	clock_t curr = clock();

	if (curr == (clock_t)(-1)) {
		YASL_pushundef(S);
	} else {
		YASL_pushfloat(S, 1.0 * curr / CLOCKS_PER_SEC);
	}

	return 1;
}

static int YASL_os_command(struct YASL_State *S) {
	if (!YASL_isstr(S) && !YASL_isundef(S)) {
		YASL_print_err(S, "TypeError: expected str or undef, got %s.", YASL_peekntypename(S, 0));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	if (YASL_isundef(S)) {
		int result = system(NULL);
		YASL_pushbool(S, result != 0);
		return 1;
	}

	char *command = YASL_popcstr(S);
	int result = system(command);
	free(command);

	YASL_pushint(S, result);
	return 1;
}

int YASL_decllib_os(struct YASL_State *S) {
	struct YASLX_function functions[] = {
		{ "getenv", &YASL_os_getenv, 1 },
		{ "command", &YASL_os_command, 1 },
		{ "clock", &YASL_os_clock, 0 },
		{ NULL, NULL, 0 }
	};

	YASL_pushtable(S);
	YASLX_tablesetfunctions(S, functions);

	YASLX_initglobal(S, "os");

	return YASL_SUCCESS;
}