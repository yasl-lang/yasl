#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_state.h"

#define VERSION_PRINTOUT "YASL " YASL_VERSION

#define YASL_LOGO " __ __  _____   ____   __   \n" \
                  "|  |  ||     | /    \\ |  |    \n" \
                  "|  |  ||  O  | |  __| |  |  \n" \
                  "|___  ||     | |__  | |  |__ \n" \
		  "|     ||  |  | |    | |     |\n" \
		  "|_____/|__|__| \\____/ |_____|\n"

// -b: run bytecode
// -c: compile to bytecode
static int main_help(int argc, char **argv) {
	(void) argc;
	(void) argv;
	puts("usage: yasl [option] [input]\n"
	     "options:\n"
	     "\t-C: checks `input` for syntax errors but doesn't run it.\n"
	     "\t-e input: executes `input` as code and prints result of last statement.\n"
	     "\t-E input: executes `input` as code.\n"
	     "\t-h: show this text.\n"
	     "\t-V: print current version.\n"
	     "\tinput: name of file containing script (or literal to execute with -e or -E)."
	);
	return 0;
}

static int main_version(int argc, char **argv) {
	(void) argc;
	(void) argv;
	puts(VERSION_PRINTOUT);
	return 0;
}

static int main_file(int argc, char **argv) {
	(void) argc;
	struct YASL_State *S = YASL_newstate(argv[1]);

	if (!S) {
		puts("ERROR: cannot open file.");
		exit(EXIT_FAILURE);
	}

	// Load Standard Libraries
	YASLX_decllibs(S);

	YASL_declglobal(S, "args");
	YASL_pushlist(S);
	for (int i = 1; i < argc; i++) {
		YASL_pushlit(S, argv[i]);
		YASL_listpush(S);
	}
	YASL_setglobal(S, "args");

	int status = YASL_execute(S);

	YASL_delstate(S);

	return status;
}

static int main_compile(int argc, char **argv) {
	(void) argc;
	struct YASL_State *S = YASL_newstate(argv[2]);

	if (!S) {
		puts("ERROR: cannot open file.");
		exit(EXIT_FAILURE);
	}

	// Load Standard Libraries
	YASLX_decllibs(S);

	int status = YASL_compile(S);
	YASL_declglobal(S, "args");
	YASL_pushlist(S);
	for (int i = 1; i < argc; i++) {
		YASL_pushlit(S, argv[i]);
		YASL_listpush(S);
	}
	YASL_setglobal(S, "args");

	YASL_delstate(S);

	return status;
}

static int main_command_REPL(int argc, char **argv) {
	(void) argc;
	const size_t size = strlen(argv[2]);
	struct YASL_State *S = YASL_newstate_bb(argv[2], size);
	YASLX_decllibs(S);
	int status = YASL_execute_REPL(S);
	YASL_delstate(S);
	return status;
}

static void YASL_quit(struct YASL_State *S) {
	(void) S;
	exit(0);
}


#ifdef __EMSCRIPTEN__
int execute_string(char *str) {
	struct YASL_State *S = YASL_newstate_bb(str, strlen(str));
	YASLX_decllibs(S);
	int status = YASL_execute(S);
	YASL_delstate(S);
	return status;
}
#endif

static int main_command(int argc, char **argv) {
	(void) argc;
	const size_t size = strlen(argv[2]);
	struct YASL_State *S = YASL_newstate_bb(argv[2], size);
	YASLX_decllibs(S);
	int status = YASL_execute(S);
	YASL_delstate(S);
	return status;
}

static int main_REPL(int argc, char **argv) {
	(void) argc;
	(void) argv;
	int next;
	struct YASL_ByteBuffer *buffer = YASL_ByteBuffer_new(8);
	struct YASL_State *S = YASL_newstate_bb((const char *)buffer->bytes, 0);
	YASLX_decllibs(S);
	YASL_declglobal(S, "quit");
	YASL_pushcfunction(S, YASL_quit, 0);
	YASL_setglobal(S, "quit");
	puts(YASL_LOGO);
	puts(VERSION_PRINTOUT);
	while (true) {
		printf("yasl> ");
		while ((next = getchar()) != '\n') {
			YASL_ByteBuffer_add_byte(buffer, next);
		}
		YASL_ByteBuffer_add_byte(buffer, '\n');

		YASL_resetstate_bb(S, (const char *)buffer->bytes, buffer->count);

		buffer->count = 0;

		YASL_execute_REPL(S);
	}
	YASL_ByteBuffer_del(buffer);
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
	srand((unsigned)time(NULL));

	if (argc == 1) {
		return main_REPL(argc, argv);
	} else if (argc == 2 && !strcmp(argv[1], "-h")) {
		return main_help(argc, argv);
	} else if (argc == 2 && !strcmp(argv[1], "-V")) {
		return main_version(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-e")) {
		return main_command_REPL(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-E")) {
		return main_command(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-C")) {
		return main_compile(argc, argv);
	} else {
		return main_file(argc, argv);
	}
}
#endif

