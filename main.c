#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "yasl.h"
#include "yasl-std-io.h"
#include "yasl-std-math.h"
#include "yasl_state.h"

#define VERSION "v0.3.6"
#define VERSION_PRINTOUT "YASL " VERSION

int main(int argc, char** argv) {
	// Initialize prng seed
	srand(time(NULL));

	if (argc > 2) {
		puts("ERROR: Too many arguments passed. Type `yasl -h` for help (without the backticks).");
		return EXIT_FAILURE;
	} else if (argc == 2) {
		if (!strcmp(argv[1], "-h")) {
			puts("usage: yasl [option] [file]\n"
			     "options:\n"
			     "\t-h: this menu\n"
			     "\t-V: print current version\n"
			     "\tfile: name of file containing script"
			);
			exit(EXIT_SUCCESS);
		} else if (!strcmp(argv[1], "-V")) {
			puts(VERSION_PRINTOUT);
			exit(EXIT_SUCCESS);
		}

		struct YASL_State *S = YASL_newstate(argv[1]);

		if (!S) {
			puts("ERROR: cannot open file.");
			exit(EXIT_FAILURE);
		}

		// Load Standard Libraries
		YASL_load_math(S);
		YASL_load_io(S);

		int status = YASL_execute(S);

		YASL_delstate(S);

		return status;
	} else {
		int next;
		size_t size = 8, count = 0;
		char *buffer = malloc(size);
		struct YASL_State *S = YASL_newstate_bb(buffer, 0);
		YASL_load_math(S);
		YASL_load_io(S);
		puts(VERSION_PRINTOUT);
		while (1) {
			printf("yasl> ");
			while ((next = getchar()) != '\n') {
				if (size == count) {
					size *= 2;
					buffer = realloc(buffer, size);
				}
				buffer[count++] = next;
			}
			if (size == count) {
				size *= 2;
				buffer = realloc(buffer, size);
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
	}

	return EXIT_SUCCESS;
}
