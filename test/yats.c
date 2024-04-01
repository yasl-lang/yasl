#include "yats.h"

#include "compiler/ast.h"
#include "compiler/compiler.h"
#include "compiler/lexinput.h"
#include "compiler/parser.h"
#include "src/yasl.h"

struct Lexer setup_lexer(const char *file_contents) {
	FILE *fptr = fopen("dump.ysl", "w");
	fwrite(file_contents, 1, strlen(file_contents), fptr);
	fseek(fptr, 0, SEEK_SET);
	fclose(fptr);
	fptr = fopen("dump.ysl", "r");
	struct LEXINPUT *lp = lexinput_new_file(fptr);
	return NEW_LEXER(lp);
}

void setup_compiler(const char *file_contents) {
	FILE *fptr = fopen("dump.ysl", "w");
	fwrite(file_contents, 1, strlen(file_contents), fptr);
	fseek(fptr, 0, SEEK_SET);
	fclose(fptr);
	fptr = fopen("dump.ysl", "r");
	struct Compiler compiler = NEW_COMPILER(lexinput_new_file(fptr));
	compiler.header->count = 24;
	unsigned char *bytecode = compile(&compiler);
	FILE *f = fopen("dump.yb", "wb");
	if (bytecode == NULL) {
		fputc(O_HALT, f);
	} else {
		fwrite(bytecode, 1, compiler.code->count + compiler.header->count + 1, f);
	}
	fclose(f);
	compiler_cleanup(&compiler);
	free(bytecode);
}

int64_t getsize(FILE *file) {
	fseek(file, 0, SEEK_END);
	int64_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}
