#pragma once

#include "compiler/lexer.h"
#include "compiler/ast.h"
#include "debug.h"

#define T1(p, a) (curtok(p) == a)
#define T2(p, a, b) T1(p, a) || T1(p, b)
#define T3(p, a, b, c) T2(p, a, b) || T1(p, c)
#define T4(p, a, b, c, d) T3(p, a, b, c) || T1(p, d)
#define CHOOSE(T1, T2, T3, T4, NAME, ...) NAME

#define TOKEN_MATCHES(parser, ...)  (CHOOSE(__VA_ARGS__, T4, T3, T2, T1, T0)(parser, __VA_ARGS__))

#define NEW_PARSER(fp)\
  ((Parser) {\
	.lex = NEW_LEXER(fp),\
	.status = YASL_SUCCESS\
  })

typedef struct {
    Lexer lex; /* OWN */
    int status;
} Parser;

int peof(const Parser *const parser);
// Parser *parser_new(FILE *fp);
void parser_cleanup(Parser *const parser);
enum Token eattok(Parser *const parser, const enum Token token);
struct Node *parse(Parser *const parser);

#ifdef _MSC_VER
#include <stdarg.h>
// To avoid MSVC _VA_ARGS_ macro expansion bug
int token_matches(Parser *const parser, ...);
#undef TOKEN_MATCHES
#define TOKEN_MATCHES(px, ...) token_matches(parser, __VA_ARGS__, (enum Token)-1)
#endif
