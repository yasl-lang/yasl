#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "yasl.h"
#include "yasl-std-io.h"
#include "yasl-std-math.h"
#include "yasl_state.h"

#define VERSION "v0.3.5"

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
            puts("YASL " VERSION);
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

        YASL_execute(S);

        YASL_delstate(S);
    } else {
	    int next;
	    size_t size = 8, count = 0;
	    struct YASL_State *S = YASL_newstate_bb("", 0);
	    YASL_load_math(S);
	    YASL_load_io(S);
	    while (1) {
		    char *buffer = malloc(size);
		    printf("YASL> ");
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

		    // printf("`%s`", buffer);
		    YASL_resetstate_bb(S, buffer, count); // *S = YASL_newstate_bb(buffer, count);

		    count = 0;
		    // Load Standard Libraries

		    YASL_execute(S);

		    // YASL_delstate(S);
	    }
    }

    return EXIT_SUCCESS;
}
