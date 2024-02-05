#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "yasl.h"
#include "yasl_aux.h"
#include "yasl_plat.h"
#include "yasl_state.h"

#define VERSION_PRINTOUT "YASL " YASL_VERSION

#define YASL_LOGO " __ __  _____   ____   __   \n" \
                  "|  |  ||     | /    \\ |  |    \n" \
                  "|  |  ||  O  | |  __| |  |  \n" \
                  "|___  ||     | |__  | |  |__ \n" \
                  "|     ||  |  | |    | |     |\n" \
                  "|_____/|__|__| \\____/ |_____|\n"

extern int random_offset;

extern int random_offset;

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

static inline void main_init_platform() {
	// Initialize prng seed
	srand(time(NULL));
	// random_offset = (size_t)rand();

	#ifdef YASL_USE_WIN
		SetConsoleOutputCP(CP_UTF8);
	#endif
}

static int main_file(int argc, char **argv) {
	YASL_ByteBuffer *buffer = YASL_ByteBuffer_new(8);
	struct YASL_State *S = YASL_newstate_bb(NULL, 0);
	// Load Standard Libraries
	YASLX_decllibs(S);

	while (argc > 1 && !strncmp(argv[1], "-D", 2)) {
		const char *equals = strchr(argv[1], '=');
		if (!equals) {
			fprintf(stderr, "Error: Expected an initial value for variable: %s\n", argv[1] + 2);
			exit(EXIT_FAILURE);
		}
		const size_t len = equals - argv[1] -2;
		if (len <= 0) {
			fprintf(stderr, "Error: Non-empty name required for variable: %s\n", argv[1] + 2);
			exit(EXIT_FAILURE);
		}
		char *name = (char *)malloc(len + 1);
		strncpy(name, argv[1] + 2, len);
		name[len] = '\0';
		YASL_declglobal(S, name);
		YASL_pushundef(S);
		YASL_setglobal(S, name);

		YASL_ByteBuffer_extend(buffer, (unsigned char *)argv[1] + 2,  strlen(argv[1]) - 2 );
		YASL_ByteBuffer_add_byte(buffer, '\n');
		YASL_resetstate_bb(S, (char *)buffer->items, buffer->count);
		buffer->count = 0;
		YASL_execute(S);

		argc--;
		argv++;
	}

	int err = YASL_resetstate(S, argv[1]);

	if (err) {
		printf("ERROR: cannot open file (%s).\n", argv[argc-1]);
		exit(EXIT_FAILURE);
	}

	YASL_declglobal(S, "args");
	YASL_pushlist(S);
	for (int i = 1; i < argc; i++) {
		YASL_pushlit(S, argv[i]);
		YASL_listpush(S);
	}
	YASL_setglobal(S, "args");

	int status = YASL_execute(S);

	YASL_delstate(S);
	YASL_ByteBuffer_del(buffer);

	return status;
}

static int main_compile(int argc, char **argv) {
	(void) argc;
	struct YASL_State *S = YASL_newstate(argv[2]);

	if (!S) {
		fprintf(stderr, "Error: cannot open file.");
		exit(EXIT_FAILURE);
	}

	// Load Standard Libraries
	YASLX_decllibs(S);

	YASL_declglobal(S, "args");
	int status = YASL_compile(S);
	YASL_pushlist(S);
	for (int i = 1; i < argc; i++) {
		YASL_pushlit(S, argv[i]);
		YASL_listpush(S);
	}
	YASL_setglobal(S, "args");

	YASL_delstate(S);

	return status;
}

static int main_gen_bytecode(int argc, char **argv) {
	(void) argc;
	struct YASL_State *S = YASL_newstate(argv[2]);

	if (!S) {
		fprintf(stderr, "Error: cannot open file.");
		exit(EXIT_FAILURE);
	}

	// Load Standard Libraries
	YASLX_decllibs(S);

	YASL_declglobal(S, "args");
	int status = YASL_compile(S);
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

static int YASL_quit(struct YASL_State *S) {
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
	YASL_ByteBuffer *buffer = YASL_ByteBuffer_new(8);
	struct YASL_State *S = YASL_newstate_bb((const char *)buffer->items, 0);
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

		YASL_resetstate_bb(S, (const char *)buffer->items, buffer->count);

		buffer->count = 0;

		YASL_execute_REPL(S);
	}
	YASL_ByteBuffer_del(buffer);
	YASL_delstate(S);
	return YASL_SUCCESS;
}

#ifdef __EMSCRIPTEN__
int main(int argc, char **argv) {
	main_init_platform();

	if (argc == 1) {
		puts(VERSION_PRINTOUT);
		return EXIT_SUCCESS;
	}
	return main_command(argc, argv);
}
#else
int main(int argc, char **argv) {
	// These don't require any state, so just return immediately.
	if (argc == 2 && !strcmp(argv[1], "-h")) {
		return main_help(argc, argv);
	} else if (argc == 2 && !strcmp(argv[1], "-V")) {
		return main_version(argc, argv);
	}

	main_init_platform();

	if (argc == 1) {
		return main_REPL(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-e")) {
		return main_command_REPL(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-E")) {
		return main_command(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-C")) {
		return main_compile(argc, argv);
	} else if (argc == 3 && !strcmp(argv[1], "-b")) {
		return main_gen_bytecode(argc, argv);
	} else {
		return main_file(argc, argv);
	}
}
#endif
