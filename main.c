#include <stdio.h>
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "compiler/compiler.h"
#include <color.h>

#include "interpreter/VM/VM.h"
#include "interpreter/builtins/builtins.h"
#include "metadata.h"

// VTable_t *float64_vtable, *int64_vtable, *str8_vtable, *list_vtable, *hash_vtable;


int main(int argc, char** argv) {
    char *buffer;
    FILE *file_ptr;
    long file_len;
    int64_t entry_point, num_globals;
    FILE *fp;
    char *name;

    if (argc > 2) {
        printf("ERROR: Too many arguments passed.\nUsage is: YASL [path/to/script.ysl]\n");
        return -1;
    } else if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            puts("Error opening file.");
            return EXIT_FAILURE;
        }
        if (!strcmp(argv[1] + strlen(argv[1]) - strlen(".ysl"), ".ysl")) {
            name = malloc(strlen(argv[1]) - strlen(".ysl") + strlen(".yb") + 1);
            memcpy(name, argv[1], strlen(argv[1]) - strlen(".ysl"));
            memcpy(name + strlen(argv[1]) - strlen(".ysl"), ".yb", strlen(".yb") + 1);
        } else if (!strcmp(argv[1] + strlen(argv[1]) - strlen(".yasl"), ".yasl")) {
            name = malloc(strlen(argv[1]) - strlen(".yasl") + strlen(".yb") + 1);
            memcpy(name, argv[1], strlen(argv[1]) - strlen(".yasl"));
            memcpy(name + strlen(argv[1]) - strlen(".yasl"), ".yb", strlen(".yb") + 1);
        } else {
            name = malloc(strlen(argv[1]) + strlen(".yb") + 1);
            memcpy(name, argv[1], strlen(argv[1]));
            memcpy(name + strlen(argv[1]), ".yb", strlen(".yb") + 1);
        }
    } else {
        puts("REPL is not yet implemented.");
        return EXIT_FAILURE;
    }

    fseek(fp, 0, SEEK_SET);
    Parser *parser = parser_new(lex_new(fp));
    Compiler *compiler = compiler_new(parser, name);
    int exit_code = compile(compiler);
    compiler_del(compiler);
    YASL_DEBUG_LOG("%s\n", "end of compilation");
    if (exit_code) {
        free(name);
        return exit_code;
    }

    file_ptr = fopen(name, "rb");
    if (file_ptr == NULL) {
        puts("Error reading bytecode file.");
        return EXIT_FAILURE;
    }

    //char magic_number[YASL_MAG_NUM_SIZE];
    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr); // - YASL_MAG_NUM_SIZE;
    rewind(file_ptr);

    /*fread(magic_number, YASL_MAG_NUM_SIZE, 1, file_ptr);
    if (strcmp("YASL", magic_number) || magic_number[4] != YASL_COMPILER ||
        magic_number[5] != YASL_MAJOR_VERSION ||
        magic_number[6] != YASL_MINOR_VERSION || magic_number[7] != YASL_PATCH) {
        puts("Invalid bytecode file.");
        exit(1);
    } */

    buffer = malloc((file_len+1)*sizeof(char));  // NOT OWN
    fread(buffer, file_len, 1, file_ptr);
    entry_point = *((int64_t*)buffer);
    num_globals = *((int64_t*)buffer+1);
    //printf("num_globals = %" PRId64 "\n", num_globals);
    // printf("entry_point = %" PRId64 "\n", entry_point);
    //bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, file_ptr);
    VM* vm = vm_new(buffer,   // program to execute
                    entry_point,    // start address of main function
                    256);   // params to be reserved, should be num_globals
    vm_run(vm);
    vm_del(vm);
    fclose(file_ptr);
    free(name);
    return 0;
};