#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "yasl.h"
#include "yasl-std-io.h"
#include "yasl-std-math.h"
#include "yasl-std-require.h"
#include "yasl_state.h"


#define VERSION "v0.4.3"
#define VERSION_PRINTOUT "YASL " VERSION

static int main_error(int argc, char **argv) {
	printf("error: Invalid arguments passed (passed %d arguments). Type `yasl -h` for help (without the backticks).\n", argc);
	return EXIT_FAILURE;
}

// -b: run bytecode
// -c: compile to bytecode
static int main_help(int argc, char **argv) {
	puts("usage: yasl [option] [input]\n"
	     "options:\n"
	     "\t-C: checks `input` for syntax errors but doesn't run it.\n"
	     "\t-e input: executes `input` as code and prints result of last statement.\n"
	     "\t-E input: executes `input` as code.\n"
	     "\t-h: show this text.\n"
	     "\t-V: print current version.\n"
	     "\tinput: name of file containing script (or literal to execute with -e or -E)."
	);
	exit(EXIT_SUCCESS);
}

static int main_version(int argc, char **argv) {
	puts(VERSION_PRINTOUT);
	exit(EXIT_SUCCESS);
}

static int main_file(int argc, char **argv) {
	if (!strcmp(argv[1], "-h")) {
		return main_help(argc, argv);
	} else if (!strcmp(argv[1], "-V")) {
		return main_version(argc, argv);
	}

	struct YASL_State *S = YASL_newstate(argv[1]);

	if (!S) {
		puts("ERROR: cannot open file.");
		exit(EXIT_FAILURE);
	}

	// Load Standard Libraries
	YASL_load_math(S);
	YASL_load_io(S);
	YASL_load_require(S);

	int status = YASL_execute(S);

	YASL_delstate(S);

	return status;
}

static int main_compile(int argc, char **argv) {
	struct YASL_State *S = YASL_newstate(argv[2]);

	if (!S) {
		puts("ERROR: cannot open file.");
		exit(EXIT_FAILURE);
	}

	// Load Standard Libraries
	YASL_load_math(S);
	YASL_load_io(S);

	int status = YASL_compile(S);

	YASL_delstate(S);

	return status;
}

static int main_command_REPL(int argc, char **argv) {
	const size_t size = strlen(argv[2]);
	struct YASL_State *S = YASL_newstate_bb(argv[2], size);
	YASL_load_math(S);
	YASL_load_io(S);
	int status = YASL_execute_REPL(S);
	YASL_delstate(S);
	return status;
}

static int main_command(int argc, char **argv) {
	const size_t size = strlen(argv[2]);
	struct YASL_State *S = YASL_newstate_bb(argv[2], size);
	YASL_load_math(S);
	YASL_load_io(S);
	int status = YASL_execute(S);
	YASL_delstate(S);
	return status;
}

static int main_REPL(int argc, char **argv) {
	int next;
	size_t size = 8, count = 0;
	char *buffer = (char *)malloc(size);
	struct YASL_State *S = YASL_newstate_bb(buffer, 0);
	YASL_load_math(S);
	YASL_load_io(S);
	puts(VERSION_PRINTOUT);
	while (1) {
		printf("yasl> ");
		while ((next = getchar()) != '\n') {
			if (size == count) {
				size *= 2;
				buffer = (char *)realloc(buffer, size);
			}
			buffer[count++] = next;
		}
		if (size == count) {
			size *= 2;
			buffer = (char *)realloc(buffer, size);
		}
		buffer[count++] = '\n';

		if (count == strlen("quit\n") && !memcmp(buffer, "quit\n", strlen("quit\n"))) {
			break;
		}
		// printf("`%s`", buffer);
		YASL_resetstate_bb(S, buffer, count); // *S = YASL_newstate_bb(buffer, count);

		count = 0;
		// Load Standard Libraries

		YASL_execute_REPL(S);
	}
	free(buffer);
	YASL_delstate(S);
	return YASL_SUCCESS;
}

#ifdef __EMSCRIPTEN__
int main(int argc, char **argv) {
	// Initialize prng seed
	srand(time(NULL));

	if (argc == 1) {
		puts(VERSION_PRINTOUT);
		return EXIT_SUCCESS;
	}
	return main_command(argc, argv);
}
#else
int main(int argc, char **argv) {
	// Initialize prng seed
	srand(time(NULL));

	if (argc == 2) {
		return main_file(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-e")) {
		return main_command_REPL(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-E")) {
		return main_command(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-C")) {
		return main_compile(argc, argv);
	} else if (argc > 2) {
		return main_error(argc, argv);
	} else {
		return main_REPL(argc, argv);
	}
}
#endif

