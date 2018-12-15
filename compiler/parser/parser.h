#pragma once

#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include "../../debug.h"

//#define peof(parser) (parser->lex->type == T_EOF)

typedef struct {
    Lexer *lex; /* OWN */
    int status;
} Parser;

int peof(const Parser *parser);
Parser *parser_new(Lexer *lex);
void parser_del(Parser *parser);
Token eattok(Parser *parser, Token token);
Node *parse(Parser *parser);