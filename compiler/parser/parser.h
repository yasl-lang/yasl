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
Parser *parser_new(Lexer *const lex);
void parser_del(Parser *const parser);
Token eattok(Parser *const parser, const Token token);
Node *parse(Parser *const parser);

static Node *parse_program(Parser *const parser);
static Node *parse_const(Parser *const parser);
static Node *parse_let(Parser *const parser);
static Node *parse_fn(Parser *const parser);
static Node *parse_block(Parser *const parser);
static Node *parse_for(Parser *const parser);
static Node *parse_while(Parser *const parser);
static Node *parse_if(Parser *const parser);
static Node *parse_expr(Parser *const parser);
static Node *parse_assign(Parser *const parser);
static Node *parse_ternary(Parser *const parser);
static Node *parse_undef_or(Parser *const parser);
static Node *parse_or(Parser *const parser);
static Node *parse_and(Parser *const parser);
static Node *parse_bor(Parser *const parser);
static Node *parse_bxor(Parser *const parser);
static Node *parse_band(Parser *const parser);
static Node *parse_equals(Parser *const parser);
static Node *parse_comparator(Parser *const parser);
static Node *parse_concat(Parser *const parser);
static Node *parse_bshift(Parser *const parser);
static Node *parse_add(Parser *const parser);
static Node *parse_multiply(Parser *const parser);
static Node *parse_unary(Parser *const parser);
static Node *parse_power(Parser *const parser);
static Node *parse_call(Parser *const parser);
static Node *parse_constant(Parser *const parser);
static Node *parse_id(Parser *const parser);
static Node *parse_undef(Parser *const parser);
static Node *parse_float(Parser *const parser);
static Node *parse_integer(Parser *const parser);
static Node *parse_boolean(Parser *const parser);
static Node *parse_string(Parser *const parser);
static Node *parse_table(Parser *const parser);
static Node *parse_collection(Parser *const parser);