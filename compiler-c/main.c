#include <stdio.h>
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "compiler.h"

int main(void) {
    FILE *fp = fopen("sample.yasl", "r");
    if (fp == NULL) return EXIT_FAILURE;
    fseek(fp, 0, SEEK_SET);
    Parser *parser = parser_new(lex_new(fp));
    Compiler *compiler = compiler_new(parser);
    compile(compiler);
    compiler_del(compiler);
}
