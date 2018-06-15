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
    // puts(K_YEL "Y" K_RED "A" K_BLU "S" K_GRN "L" K_END "\n");
    char *buffer;
    FILE *file_ptr;
    long file_len;
    int64_t entry_point, num_globals;

    /*if (argc > 2) {
        printf("ERROR: Too many arguments passed.\nUsage is: YASL [path/to/byte-code.yb]\nDefault path is \"source.yb\"\n");
        return -1;
    } else if (argc == 2) {
        file_ptr = fopen(argv[1], "rb");
        if(file_ptr == NULL) {
            printf("ERROR: Could not open source file \"%s\".\n", argv[1]);
            return -2;
        }
    } else */{
        //printf("YASL> ");
        //Compiler *compiler = compiler_new(parser_new(lex_new(stdin)));
        //compile(compiler);
        //compiler_del(compiler);

        file_ptr = fopen("source.yb", "rb");
        if(file_ptr == NULL) {
            printf("ERROR: Could not default open source file \"source.yb\".\n");
            return -3;
        }
    }

    FILE *fp = fopen("sample.ysl", "r");
    if (fp == NULL) return EXIT_FAILURE;
    fseek(fp, 0, SEEK_SET);
    Parser *parser = parser_new(lex_new(fp));
    Compiler *compiler = compiler_new(parser);
    compile(compiler);
    compiler_del(compiler);
    YASL_DEBUG_LOG("%s\n", "end of compilation");

    char magic_number[YASL_MAG_NUM_SIZE];
    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr) - YASL_MAG_NUM_SIZE;
    rewind(file_ptr);

    fread(magic_number, YASL_MAG_NUM_SIZE, 1, file_ptr);
    if (strcmp("YASL", magic_number) || magic_number[4] != YASL_COMPILER ||
        magic_number[5] != YASL_MAJOR_VERSION ||
        magic_number[6] != YASL_MINOR_VERSION || magic_number[7] != YASL_PATCH) {
        puts("Invalid bytecode file.");
        exit(1);
    }

    buffer = malloc((file_len+1)*sizeof(char));
    fread(buffer, file_len, 1, file_ptr);
    entry_point = *((int64_t*)buffer);
    num_globals = *((int64_t*)buffer+1);
    //printf("num_globals = %" PRId64 "\n", num_globals);
    // printf("entry_point = %" PRId64 "\n", entry_point);
    //bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, file_ptr);
    VM* vm = newVM(buffer,   // program to execute
                   entry_point,    // start address of main function
                   256);   // locals to be reserved, should be num_globals
    run(vm);
    delVM(vm);
    free(buffer);
    fclose(file_ptr);
    return 0;
};