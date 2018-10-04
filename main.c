#include <stdio.h>
#include <stdlib.h>
#include "yasl.h"

int main(int argc, char** argv) {
    if (argc > 2) {
        printf("ERROR: Too many arguments passed.\nUsage is: YASL [path/to/script.ysl]\n");
        return EXIT_FAILURE;
    } else if (argc == 2) {
        struct YASL_State *S = YASL_newstate(argv[1]);

        YASL_execute(S);

        YASL_delstate(S);
    } else {
        puts("REPL is not yet implemented.");
        return EXIT_FAILURE;
    }

    return 0;
};
