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
    T_UNKNOWN = -1,
    T_EOF,
    T_SEMI,
    T_UNDEF,
    T_FLOAT64,
    T_INT64,
    T_BOOL,
    T_STR,
    T_IF,
    T_ELSEIF,
    T_ELSE,
    T_WHILE,
    T_BREAK,
    T_CONT,
    T_FOR,
    T_AND,
    T_OR,
    T_ID,
    T_LET,
    T_FN,
    T_RET,
    T_STRUCT,
    T_PRINT,
    T_LPAR,
    T_RPAR,
    T_LSQB,
    T_RSQB,
    T_LBRC,
    T_RBRC,
    T_DOT,
    T_COMMA,
    T_CARET,
    T_CARETEQ,
    T_PLUS,
    T_PLUSEQ,
    T_MINUS,
    T_MINUSEQ,
    T_HASH,
    T_BANG,
    T_BANGEQ,
    T_BANGDEQ,
    T_TILDE,
    T_TILDEEQ,
    T_STAR,
    T_STAREQ,
    T_DSTAR,
    T_DSTAREQ,
    T_SLASH,
    T_SLASHEQ,
    T_DSLASH,
    T_DSLASHEQ,
    T_MOD,
    T_MODEQ,
    T_LT,
    T_LTEQ,
    T_DLT,
    T_DLTEQ,
    T_GT,
    T_GTEQ,
    T_DGT,
    T_DGTEQ,
    T_EQ,
    T_DEQ,
    T_TEQ,
    T_AMP,
    T_AMPEQ,
    T_BAR,
    T_BAREQ,
    T_DBAR,
    T_DBAREQ,
    T_TBAR,
    T_TBAREQ,
    T_QMARK,
    T_DQMARK,
    T_DQMARKEQ,
    T_COLON,
    T_RARR,
    T_LARR,
} Token;