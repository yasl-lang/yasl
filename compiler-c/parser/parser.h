#pragma once

#include "../lexer/lexer.h"
#include "../ast/ast.h"
#define peof(parser) (parser->lex->type == TOK_EOF)
#define isaugmented(t) (t == TOK_CARETEQ || t == TOK_STAREQ || t == TOK_SLASHEQ || t == TOK_DSLASHEQ ||\
            t == TOK_MOD || t == TOK_PLUSEQ || t == TOK_MINUSEQ || t == TOK_DGTEQ || t == TOK_DLTEQ || \
            t == TOK_DBAREQ || t == TOK_TBAREQ || t == TOK_AMPEQ || t == TOK_TILDEEQ || t == TOK_BAREQ || \
            t == TOK_DQMARKEQ)
// ^=, *=, /=, //=,
// %=, +=, -=, >>=, <<=,
// ||=, |||=, &=, ~=, |=,
// ?\?=

typedef struct {
    Lexer *lex;
} Parser;

Parser *parser_new(Lexer *lex);
void parser_del(Parser *parser);
Token eattok(Parser *parser, Token token);

Node *parse(Parser *parser);
Node *parse_program(Parser *parser);
Node *parse_let(Parser *parser);
Node *parse_while(Parser *parser);
Node *parse_if(Parser *parser);
Node *parse_expr(Parser *parser);
Node *parse_assign(Parser *parser);
Node *parse_ternary(Parser *parser);
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
Node *parse_collection(Parser *parser);