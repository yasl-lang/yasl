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
    if (parser->lex->type == TOK_PRINT) {
        gettok(parser->lex);
        return new_Print(expr(parser));
    }
    puts("ParsingError: Unknown sequence.");
    exit(EXIT_FAILURE);
}

Node *expr(Parser *parser) {
    if (parser->lex->type == TOK_STR) return string(parser);
    else if (parser->lex->type == TOK_INT64) return integer(parser);
    puts("ParsingError: Invalid exdpresion.");
    exit(EXIT_FAILURE);
}

Node *integer(Parser *parser) {
    return new_Integer(parser->lex->value, parser->lex->val_len);
}

Node *string(Parser *parser) {
    //gettok(parser->lex);
    return new_String(parser->lex->value, parser->lex->val_len);
}