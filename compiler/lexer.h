#pragma once

#include "token.h"
#include "lexinput.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define  ispotentialend(l) ((l)->type == T_ID || (l)->type == T_STR || \
            (l)->type == T_INT || (l)->type == T_FLOAT || (l)->type == T_BREAK || \
            (l)->type == T_CONT || (l)->type == T_RPAR || (l)->type == T_RSQB || \
            (l)->type == T_RBRC || (l)->type == T_UNDEF || (l)->type == T_BOOL)

#ifndef NEW_FOO
#ifndef __cplusplus
#define NEW_FOO(a, ...) ((a) __VA_ARGS__)
#else
#define NEW_FOO(a, ...) __VA_ARGS__
#endif
#endif

#define NEW_LEXER(f) \
  NEW_FOO(Lexer, { .file = (f),\
    .c = 0,\
	   .type = T_UNKNOWN,\
	   .value = NULL,\
	   .val_len = 0,\
	   .line = 1,\
	   .status = YASL_SUCCESS,\
	   .mode = L_NORMAL,\
})

#define STR_DELIM '\''
#define RAW_STR_DELIM '`'
#define INTERP_STR_DELIM '"'
#define INTERP_STR_PLACEHOLDER '#'
#define NUM_SEPERATOR '_'

enum LexerModes {
    L_NORMAL,     // default mode.
    L_INTERP,     // for string interpolation.
};

typedef struct {
    struct LEXINPUT *file;     // OWN
    char c;
    enum Token type;
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
int lex_eatfloatexp(Lexer *lex);

extern const char *YASL_TOKEN_NAMES[84];
