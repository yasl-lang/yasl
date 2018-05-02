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
Node *parse_program(Parser *parser);
Node *parse_expr(Parser *parser);
Node *parse_or(Parser *parser);
Node *parse_and(Parser *parser);
Node *parse_bor(Parser *parser);
Node *parse_bxor(Parser *parser);
Node *parse_band(Parser *parser);
Node *parse_equals(Parser *parser);
Node *parse_comparator(Parser *parser);
Node *parse_concat(Parser *parser);
Node *parse_bshift(Parser *parser);
Node *parse_add(Parser *parser);
Node *parse_multiply(Parser *parser);
Node *parse_unary(Parser *parser);
Node *parse_power(Parser *parser);
Node *parse_constant(Parser *parser);
Node *parse_id(Parser *parser);
Node *parse_undef(Parser *parser);
Node *parse_float(Parser *parser);
Node *parse_integer(Parser *parser);
Node *parse_boolean(Parser *parser);
Node *parse_string(Parser *parser);