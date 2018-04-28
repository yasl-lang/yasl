#include <stdio.h>
#include "parser/parser.h"
#include "lexer/lexer.h"

int main(void) {
    FILE *fp = fopen("sample.yasl", "r");
    if (fp == NULL) return EXIT_FAILURE;
    fseek(fp, 0, SEEK_SET);
    Parser *parser = parser_new(lex_new(fp));
    while (parser->lex->type != TOK_EOF) {
            gettok(parser->lex);
            printf("type: %s, value: %s\n", YASL_TOKEN_NAMES[parser->lex->type], parser->lex->value);
    }
    parser_del(parser);
}
