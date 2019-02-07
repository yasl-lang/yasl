#pragma once

#include "../token.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define  ispotentialend(l) ((l)->type == T_ID || (l)->type == T_STR || \
            (l)->type == T_INT || (l)->type == T_FLOAT || (l)->type == T_BREAK || \
            (l)->type == T_CONT || (l)->type == T_RPAR || (l)->type == T_RSQB || \
            (l)->type == T_RBRC || (l)->type == T_UNDEF || (l)->type == T_BOOL)

#define NEW_LEXER(f) \
((Lexer) { .file = (f),\
	   .line = 1,\
	   .value = NULL,\
	   .val_len = 0,\
	   .type = T_UNKNOWN,\
	   .status = YASL_SUCCESS,\
	   .mode = L_NORMAL\
})

/*
an identifier
an integer, floating-point, or string literal
one of the keywords break, or continue
one of the delimiters ), ], or }
*/
#define STR_DELIM '\''
#define RAW_STR_DELIM '`'
#define INTERP_STR_DELIM '"'
#define INTERP_STR_PLACEHOLDER '#'

enum LexerModes {
    L_NORMAL,     // default mode.
    L_INTERP,     // for string interpolation.
};

typedef struct {
    FILE *file;     // OWN
    char c;
    Token type;
    char *value;    // NOT OWN
    size_t val_len;
    size_t line;
    int status;
    int mode;
} Lexer;

// Lexer *lex_new(FILE *file);
void lex_cleanup(Lexer *lex);
void gettok(Lexer *lex);
int lex_eatinterpstringbody(Lexer *lex);

const char *YASL_TOKEN_NAMES[84];