#pragma once

#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include "../../debug.h"

//#define peof(parser) (parser->lex->type == T_EOF)

typedef struct {
    Lexer *lex; /* OWN */
} Parser;

int peof(const Parser *parser);
Parser *parser_new(Lexer *const lex);
void parser_del(Parser *const parser);
Token eattok(const Parser *const parser, const Token token);
Node *parse(const Parser *const parser);

static Node *parse_program(const Parser *const parser);
static Node *parse_const(const Parser *const parser);
static Node *parse_let(const Parser *const parser);
static Node *parse_fn(const Parser *const parser);
static Node *parse_block(const Parser *const parser);
static Node *parse_for(const Parser *const parser);
static Node *parse_while(const Parser *const parser);
static Node *parse_if(const Parser *const parser);
static Node *parse_expr(const Parser *const parser);
static Node *parse_assign(const Parser *const parser);
static Node *parse_ternary(const Parser *const parser);
static Node *parse_or(const Parser *const parser);
static Node *parse_and(const Parser *const parser);
static Node *parse_bor(const Parser *const parser);
static Node *parse_bxor(const Parser *const parser);
static Node *parse_band(const Parser *const parser);
static Node *parse_equals(const Parser *const parser);
static Node *parse_comparator(const Parser *const parser);
static Node *parse_concat(const Parser *const parser);
static Node *parse_bshift(const Parser *const parser);
static Node *parse_add(const Parser *const parser);
static Node *parse_multiply(const Parser *const parser);
static Node *parse_unary(const Parser *const parser);
static Node *parse_power(const Parser *const parser);
static Node *parse_call(const Parser *const parser);
static Node *parse_constant(const Parser *const parser);
static Node *parse_id(const Parser *const parser);
static Node *parse_undef(const Parser *const parser);
static Node *parse_float(const Parser *const parser);
static Node *parse_integer(const Parser *const parser);
static Node *parse_boolean(const Parser *const parser);
static Node *parse_string(const Parser *const parser);
static Node *parse_table(const Parser *const parser);
static Node *parse_collection(const Parser *const parser);