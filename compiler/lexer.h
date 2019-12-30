#pragma once

#include <stdbool.h>

#include "IO.h"
#include "lexinput.h"
#include "token.h"

#define  ispotentialend(l) ((l)->type == T_ID || (l)->type == T_STR || \
            (l)->type == T_INT || (l)->type == T_FLOAT || (l)->type == T_BREAK || \
            (l)->type == T_CONT || (l)->type == T_RPAR || (l)->type == T_RSQB || \
            (l)->type == T_RBRC || (l)->type == T_UNDEF || (l)->type == T_BOOL)

#define NEW_LEXER(f) \
  ((struct Lexer) { .file = (f),\
             .c = 0,\
             .type = T_UNKNOWN,\
             .value = NULL,\
             .val_cap = 0,\
             .val_len = 0,\
             .line = 1,\
             .status = YASL_SUCCESS,\
             .mode = L_NORMAL,\
             .err = ((struct IO) { io_print_file, stderr, NULL, 0 })\
})

#define ESCAPE_CHAR '\\'
#define STR_DELIM '\''
#define RAW_STR_DELIM '`'
#define INTERP_STR_DELIM '"'
#define INTERP_STR_PLACEHOLDER '#'
#define NUM_SEPERATOR '_'

enum LexerModes {
    L_NORMAL,     // default mode.
    L_INTERP,     // for string interpolation.
};

struct Lexer {
    struct LEXINPUT *file;   // OWN
    char c;
    enum Token type;
    char *value;             // NOT OWN
    size_t val_cap;
    size_t val_len;
    size_t line;
    int status;
    int mode;
    struct IO err;
};

// struct Lexer *lex_new(FILE *file);
void lex_cleanup(struct Lexer *lex);
void gettok(struct Lexer *lex);
void lex_eatinterpstringbody(struct Lexer *lex);
void lex_error(struct Lexer *lex);
int lex_getchar(struct Lexer *lex);

extern const char *YASL_TOKEN_NAMES[86];
