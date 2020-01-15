#pragma once

#include "lexer.h"

#define T1(p, a) (curtok(p) == a)
#define T2(p, a, b) T1(p, a) || T1(p, b)
#define T3(p, a, b, c) T2(p, a, b) || T1(p, c)
#define T4(p, a, b, c, d) T3(p, a, b, c) || T1(p, d)
#define CHOOSE(T1, T2, T3, T4, NAME, ...) NAME

#define TOKEN_MATCHES(parser, ...)  (CHOOSE(__VA_ARGS__, T4, T3, T2, T1, T0)(parser, __VA_ARGS__))

#define NEW_PARSER(fp)\
  ((struct Parser) {\
	.lex = NEW_LEXER(fp),\
	.status = YASL_SUCCESS\
  })

struct Parser {
    struct Lexer lex; /* OWN */
    int status;
};

struct Parser new_parser(struct LEXINPUT *lp) {
	struct Parser p = ((struct Parser) {
		//.lex = new_lexer(lp),
		.status = 0
	});
	p.lex = new_lexer(lp);
	return p;
}

int peof(const struct Parser *const parser);
// struct Parser *parser_new(FILE *fp);
void parser_cleanup(struct Parser *const parser);
enum Token eattok(struct Parser *const parser, const enum Token token);
struct Node *parse(struct Parser *const parser);

#ifdef _MSC_VER
#include <stdarg.h>
// To avoid MSVC _VA_ARGS_ macro expansion bug
int token_matches(struct Parser *const parser, ...);
#undef TOKEN_MATCHES
#define TOKEN_MATCHES(px, ...) token_matches(parser, __VA_ARGS__, (enum Token)-1)
#endif
