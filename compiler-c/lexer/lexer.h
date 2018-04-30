#pragma once

#include "../token.h"
// #include "../bytebuffer.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define  isbdigit(x) (x == '0' || x == '1')
#define  isodigit(x) (isdigit(x) && x != '8' && x != '9')
#define  ispotentialend(l) (l->type == TOK_ID || l->type == TOK_STR || \
            l->type == TOK_INT64 || l->type == TOK_FLOAT64 || l->type == TOK_BREAK || \
            l->type == TOK_CONT || l->type == TOK_RPAR || l->type == TOK_RSQB || \
            l->type == TOK_RBRC)
/*
an identifier
an integer, floating-point, or string literal
one of the keywords break, or continue
one of the delimiters ), ], or }
*/

typedef struct {
    FILE *file;
    Token type;
    char *value;
    int val_len;
    int line;
} Lexer;

Token YASLToken_FourChars(char c1, char c2, char c3, char c4);
Token YASLToken_ThreeChars(char c1, char c2, char c3);
Token YASLToken_TwoChars(char c1, char c2);
Token YASLToken_OneChar(char c1);
void YASLKeywords(Lexer *lex);

Lexer *lex_new(FILE *file);
void lex_del(Lexer *lex);
void gettok(Lexer *lex);

const char *YASL_TOKEN_NAMES[600/8];