#pragma once

#include "../token.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define  ispotentialend(l) (l->type == T_ID || l->type == T_STR || \
            l->type == T_INT64 || l->type == T_FLOAT64 || l->type == T_BREAK || \
            l->type == T_CONT || l->type == T_RPAR || l->type == T_RSQB || \
            l->type == T_RBRC || l->type == T_UNDEF || l->type == T_BOOL)



/*
an identifier
an integer, floating-point, or string literal
one of the keywords break, or continue
one of the delimiters ), ], or }
*/
#define STR_DELIM '\''

typedef struct {
    FILE *file;
    char c;
    Token type;
    char *value;
    int64_t val_len;
    int line;
} Lexer;

static Token YASLToken_FourChars(char c1, char c2, char c3, char c4);
static Token YASLToken_ThreeChars(char c1, char c2, char c3);
static Token YASLToken_TwoChars(char c1, char c2);
static Token YASLToken_OneChar(char c1);
static void YASLKeywords(Lexer *lex);

Lexer *lex_new(FILE *file);
void lex_del(Lexer *lex);
void gettok(Lexer *lex);

const char *YASL_TOKEN_NAMES[80];