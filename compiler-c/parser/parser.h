#pragma once

#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include "../../debug.h"

#define peof(parser) (parser->lex->type == T_EOF)
#define isaugmented(t) (t == T_CARETEQ || t == T_STAREQ || t == T_SLASHEQ || t == T_DSLASHEQ ||\
            t == T_MOD || t == T_PLUSEQ || t == T_MINUSEQ || t == T_DGTEQ || t == T_DLTEQ || \
            t == T_DBAREQ || t == T_TBAREQ || t == T_AMPEQ || t == T_DSTAREQ || t == T_BAREQ || \
            t == T_DQMARKEQ)
// ^=, *=, /=, //=,
// %=, +=, -=, >>=, <<=,
// ||=, |||=, &=, **=, |=,
// ?\?=

#define curtok(p) (p->lex->type)

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
Node *parse_call(Parser *parser);
Node *parse_constant(Parser *parser);
Node *parse_id(Parser *parser);
Node *parse_undef(Parser *parser);
Node *parse_float(Parser *parser);
Node *parse_integer(Parser *parser);
Node *parse_boolean(Parser *parser);
Node *parse_string(Parser *parser);
Node *parse_collection(Parser *parser);