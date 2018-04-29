#pragma once

#include "../lexer/lexer.h"
#include "../ast/ast.h"
#define peof(parser) (parser->lex->type == TOK_EOF)

typedef struct {
    Lexer *lex;
} Parser;

Parser *parser_new(Lexer *lex);
void parser_del(Parser *parser);

Node *parse(Parser *parser);
Node *program(Parser *parser);
Node *expr(Parser *parser);
Node *string(Parser *parser);