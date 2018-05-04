#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* OneCharTokens:
 * LPAREN (
 * RPAREN )
 * LBRACK [
 * RBRACK ]
 * LBRACE {
 * RBRACE }
 * DOT    .
 * CARET  ^
 * PLUS   +
 * MINUS  -
 * HASH   #
 * BANG   !
 * TILDE  ~
 * STAR   *
 * SLASH  /
 * MOD    %
 * LT     <
 * GT     >
 * EQ     =
 * AMP    &
 * BAR    |
 * QMARK  ?
 * COLON  :
 */

/* TwoCharTokens:
 * CARETEQ    ^=
 * PLUSEQ     +=
 * MINUSEQ    -=
 * BANGEQ     !=
 * DEQ        ==
 * TILDEEQ    ~=
 * STAREQ     *=
 * SLASHEQ    /=
 * MODEQ      %=
 * LTLT       <<
 * GTGT       >>
 * LTEQ       <=
 * GTEQ       >=
 * AMPEQ      &=
 * BAREQ      |=
 * DBAR       ||
 * DQMARK     ??
 * RARROW     ->
 * LARROW     <-
 */

/* ThreeCharTokens:
 * DLTEQ    <<=
 * DGTEQ    >>=
 * TEQ      ===
 * BANGDEQ  !==
 * DSLASH   //=
 * DBAREQ   ||=
 * TBAR     |||
 * DQMARKEQ ??=
 */

/* FourCharTokens:
 * TBAREQ   |||=
 */


// NOTE: make sure that augmented version of operand is directly after regular version for all operants.
// parser.c uses this
typedef enum {
    UNKNOWN = -1,
    TOK_EOF,
    TOK_SEMI,
    TOK_UNDEF,
    TOK_FLOAT64,
    TOK_INT64,
    TOK_BOOL,
    TOK_STR,
    TOK_IF,
    TOK_ELSEIF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_BREAK,
    TOK_CONT,
    TOK_FOR,
    TOK_AND,
    TOK_OR,
    TOK_ID,
    TOK_LET,
    TOK_FN,
    TOK_RET,
    TOK_STRUCT,
    TOK_PRINT,
    TOK_LPAR,
    TOK_RPAR,
    TOK_LSQB,
    TOK_RSQB,
    TOK_LBRC,
    TOK_RBRC,
    TOK_DOT,
    TOK_COMMA,
    TOK_CARET,
    TOK_CARETEQ,
    TOK_PLUS,
    TOK_PLUSEQ,
    TOK_MINUS,
    TOK_MINUSEQ,
    TOK_HASH,
    TOK_BANG,
    TOK_BANGEQ,
    TOK_BANGDEQ,
    TOK_TILDE,
    TOK_TILDEEQ,
    TOK_STAR,
    TOK_STAREQ,
    TOK_SLASH,
    TOK_SLASHEQ,
    TOK_DSLASH,
    TOK_DSLASHEQ,
    TOK_MOD,
    TOK_MODEQ,
    TOK_LT,
    TOK_LTEQ,
    TOK_DLT,
    TOK_DLTEQ,
    TOK_GT,
    TOK_GTEQ,
    TOK_DGT,
    TOK_DGTEQ,
    TOK_EQ,
    TOK_DEQ,
    TOK_TEQ,
    TOK_AMP,
    TOK_AMPEQ,
    TOK_BAR,
    TOK_BAREQ,
    TOK_DBAR,
    TOK_DBAREQ,
    TOK_TBAR,
    TOK_TBAREQ,
    TOK_QMARK,
    TOK_DQMARK,
    TOK_DQMARKEQ,
    TOK_COLON,
    TOK_RARR,
    TOK_LARR,
} Token;