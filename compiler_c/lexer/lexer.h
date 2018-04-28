#pragma once

#include "../token.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define  isbdigit(x) (x == '0' || x == '1')
// #define EATWHITESPACE() while (lastchar == ' ' || lastchar == '\t') { lastchar = getchar(); }

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

const char *YASL_TOKEN_NAMES[];