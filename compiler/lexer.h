#ifndef YASL_LEXER_H_
#define	YASL_LEXER_H_

#include <stdbool.h>

#include "IO.h"
#include "lexinput.h"
#include "data-structures/YASL_ByteBuffer.h"

#define  ispotentialend(l) ((l)->type == T_ID || (l)->type == T_STR || \
            (l)->type == T_INT || (l)->type == T_FLOAT || (l)->type == T_BREAK || \
            (l)->type == T_CONT || (l)->type == T_RPAR || (l)->type == T_RSQB || \
            (l)->type == T_RBRC || (l)->type == T_UNDEF || (l)->type == T_BOOL)

#define NEW_LEXER(f) ((struct Lexer) {\
        .file = (f),\
        .c = 0,\
        .type = T_UNKNOWN,\
        .buffer.size = 0,\
        .buffer.count = 0,\
        .buffer.bytes = NULL,\
        .line = 1,\
        .status = YASL_SUCCESS,\
        .mode = L_NORMAL,\
        .err = NEW_IO(stderr)\
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
#define X(E, ...) E,
#include "tokentype.x"
#undef X
};

struct Lexer {
	struct LEXINPUT *file;   // OWN
	int c;                   // current character
	enum Token type;         // type of current token
	struct YASL_ByteBuffer buffer;
	size_t line;
	int status;
	int mode;
	struct IO err;
};

void lex_cleanup(struct Lexer *const lex);
void gettok(struct Lexer *const lex);
void lex_eatinterpstringbody(struct Lexer *const lex);
void lex_error(struct Lexer *const lex);
int lex_getchar(struct Lexer *const lex);

void lex_val_setnull(struct Lexer *const lex);
void lex_val_free(struct Lexer *const lex);
char *lex_val_get(struct Lexer *const lex);
void lex_val_set(struct Lexer *const lex, char *val);

extern const char *YASL_TOKEN_NAMES[84];

#endif
