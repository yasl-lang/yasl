#include "yats.h"

#include "yasl.h"
#include "ast.h"
#include "parser.h"
#include "compiler.h"
#include "yats.h"
#include "test/test_compiler/compilertest.h"

Lexer setup_lexer(char *file_contents) {
	FILE *fptr = fopen("dump.ysl", "w");
	fwrite(file_contents, 1, strlen(file_contents), fptr);
	fseek(fptr, 0, SEEK_SET);
	fclose(fptr);
	fptr = fopen("dump.ysl", "r");
	return NEW_LEXER(fptr);
}


unsigned char *setup_compiler(char *file_contents) {
    FILE *fptr = fopen("dump.ysl", "w");
    fwrite(file_contents, 1, strlen(file_contents), fptr);
    fseek(fptr, 0, SEEK_SET);
    fclose(fptr);
    fptr = fopen("dump.ysl", "r");
    struct Compiler *compiler = compiler_new(fptr);
    unsigned char *bytecode = compile(compiler);
    FILE *f = fopen("dump.yb", "wb");
    if (bytecode == NULL) {
        fputc(HALT, f);
    } else {
        fwrite(bytecode, 1, compiler->code->count + compiler->header->count + 1, f);
    }
    fclose(f);
    compiler_del(compiler);
    return bytecode;
}

int64_t getsize(FILE *file) {
    fseek(file, 0, SEEK_END);
    int64_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}