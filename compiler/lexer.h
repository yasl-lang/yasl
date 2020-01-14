#pragma once

#include <stdbool.h>

#include "IO.h"
#include "lexinput.h"

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

// NOTE: make sure that augmented version of operand is directly after regular version for all operands.
// parser.c uses this
// NOTE: keep up to date with array of names (YASL_TOKEN_NAMES) in lexer.c
enum Token {
	T_UNKNOWN = -1,
	T_EOF,        // END OF FILE
	T_SEMI,       // ;
	T_UNDEF,      // undef
	T_FLOAT,      // FLOAT
	T_INT,        // INT
	T_BOOL,       // BOOL
	T_STR,        // STR
	T_IF,         // if
	T_ELSEIF,     // elseif
	T_ELSE,       // else
	T_WHILE,      // while
	T_BREAK,      // break
	T_CONT,       // continue
	T_FOR,        // for
	T_ID,         // ID
	T_CONST,      // const
	T_FN,         // fn
	T_LET,        // let
	T_RET,        // return
	T_EXPORT,     // export
	T_ECHO,       // echo
	T_LEN,        // len
	T_LPAR,       // (
	T_RPAR,       // )
	T_LSQB,       // [
	T_RSQB,       // ]
	T_LBRC,       // {
	T_RBRC,       // }
	T_DOT,        // .
	T_TDOT,       // ...
	T_COMMA,      // ,
	T_CARET,      // ^
	T_CARETEQ,    // ^=
	T_PLUS,       // +
	T_PLUSEQ,     // +=
	T_MINUS,      // -
	T_MINUSEQ,    // -=
	T_BANG,       // !
	T_BANGEQ,     // !=
	T_BANGDEQ,    // !==
	T_BANGTILDE,  // !~
	T_TILDE,      // ~
	T_TILDEEQ,    // ~=
	T_STAR,       // *
	T_STAREQ,     // *=
	T_DSTAR,      // **
	T_DSTAREQ,    // **=
	T_SLASH,      // /
	T_SLASHEQ,    // /=
	T_DSLASH,     // //
	T_DSLASHEQ,   // //=
	T_MOD,        // %
	T_MODEQ,      // %=
	T_LT,         // <
	T_LTEQ,       // <=
	T_DLT,        // <<
	T_DLTEQ,      // <<=
	T_GT,         // >
	T_GTEQ,       // >=
	T_DGT,        // >>
	T_DGTEQ,      // >>=
	T_EQ,         // =
	T_DEQ,        // ==
	T_TEQ,        // ===
	T_EQTILDE,    // =~
	T_AMP,        // &
	T_AMPEQ,      // &=
	T_DAMP,       // &&
	T_DAMPEQ,     // &&=
	T_AMPCARET,   // &^
	T_AMPCARETEQ, // &^=
	T_BAR,        // |
	T_BAREQ,      // |=
	T_DBAR,       // ||
	T_DBAREQ,     // ||=
	T_QMARK,      // ?
	T_DQMARK,     // ??
	T_DQMARKEQ,   // ??=
	T_COLON,      // :
	T_COLONEQ,    // :=
	T_RIGHT_ARR,  // ->
	T_LEFT_ARR,   // <-
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

struct Lexer new_lex(struct LEXINPUT *f) {
    struct Lexer lex = ((struct Lexer) {
             .file = f,
             .c = 0,
             .type = T_UNKNOWN,
             .value = NULL,
             .val_cap = 0,
             .val_len = 0,
             .line = 1,
             .status = YASL_SUCCESS,
             .mode = L_NORMAL,
             .err = ((struct IO) { io_print_file, stderr, NULL, 0 })
    })
    return lex;
}

void lex_cleanup(struct Lexer *const lex);
void gettok(struct Lexer *const lex);
void lex_eatinterpstringbody(struct Lexer *const lex);
void lex_error(struct Lexer *const lex);
int lex_getchar(struct Lexer *const lex);

extern const char *YASL_TOKEN_NAMES[82];

