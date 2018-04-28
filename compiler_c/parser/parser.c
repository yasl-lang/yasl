#include "parser.h"

Parser *parser_new(Lexer *lex) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lex = lex;
    return parser;
};

void parser_del(Parser *parser) {
    lex_del(parser->lex);
    free(parser);
};

Node *parse(Parser *parser) {
    return program(parser);
}

Node *program(Parser *parser) {
    gettok(parser->lex);
    if (parser->lex->type == TOK_PRINT) {
        gettok(parser->lex);
        return(new_Print(expr(parser)));
    }
}

Node *expr(Parser *parser) {

}