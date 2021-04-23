#ifndef YASL_PARSER_H_
#define YASL_PARSER_H_

#include <setjmp.h>
#include "lexer.h"
#include "yapp.h"
#include "ast.h"

#define T1(p, a) (curtok(p) == a)
#define T2(p, a, b) T1(p, a) || T1(p, b)
#define T3(p, a, b, c) T2(p, a, b) || T1(p, c)
#define T4(p, a, b, c, d) T3(p, a, b, c) || T1(p, d)

#define TOKEN_MATCHES(parser, ...)  (YAPP_EXPAND(YAPP_CHOOSE4(__VA_ARGS__, T4, T3, T2, T1)(parser, __VA_ARGS__)))

#define NEW_PARSER(fp)\
  ((struct Parser) {\
	.lex = NEW_LEXER(fp),\
	.status = YASL_SUCCESS\
  })

struct Parser {
	struct Lexer lex; /* OWN */
	int status;
	jmp_buf env;
};

int peof(const struct Parser *const parser);
size_t parserline(const struct Parser *const parser);
void parser_cleanup(struct Parser *const parser);
enum Token eattok(struct Parser *const parser, const enum Token token);
struct Node *parse(struct Parser *const parser);

#endif
