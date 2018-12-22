#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yasl.h"
#include "yasl-std-io.h"

#define VERSION "v0.2.1"

int main(int argc, char** argv) {
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

       // YASL_load_io(S);

        YASL_execute(S);

        YASL_delstate(S);
    } else {
        puts("REPL is not yet implemented.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
