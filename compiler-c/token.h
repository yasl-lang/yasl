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
    TOK_PLUS,
    TOK_MINUS,
    TOK_HASH,
    TOK_BANG,
    TOK_TILDE,
    TOK_STAR,
    TOK_SLASH,
    TOK_MOD,
    TOK_LT,
    TOK_GT,
    TOK_EQ,
    TOK_AMP,
    TOK_BAR,
    TOK_QMARK,
    TOK_COLON,
    TOK_CARETEQ,
    TOK_PLUSEQ,
    TOK_MINUSEQ,
    TOK_BANGEQ,
    TOK_DEQ,
    TOK_TILDEEQ,
    TOK_STAREQ,
    TOK_SLASHEQ,
    TOK_MODEQ,
    TOK_DLT,
    TOK_DGT,
    TOK_LTEQ,
    TOK_GTEQ,
    TOK_AMPEQ,
    TOK_BAREQ,
    TOK_DBAR,
    TOK_DQMARK,
    TOK_RARR,
    TOK_LARR,
    TOK_DLTEQ,
    TOK_DGTEQ,
    TOK_TEQ,
    TOK_BANGDEQ,
    TOK_DSLASHEQ,
    TOK_DBAREQ,
    TOK_TBAR,
    TOK_DQMARKEQ,
    TOK_TBAREQ,
} Token;