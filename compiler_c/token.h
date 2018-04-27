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
 * TBAREQ |||=
 */


enum Tokens {
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
    LPAR,
    RPAR,
    LSQB,
    RSQB,
    LBRC,
    RBRC,
    DOT,
    COMMA,
    CARET,
    PLUS,
    MINUS,
    HASH,
    BANG,
    TILDE,
    STAR,
    SLASH,
    MOD,
    LT,
    GT,
    EQ,
    AMP,
    BAR,
    QMARK,
    COLON,
    CARETEQ,
    PLUSEQ,
    MINUSEQ,
    BANGEQ,
    DEQ,
    TILDEEQ,
    STAREQ,
    SLASHEQ,
    MODEQ,
    DLT,
    DGT,
    LTEQ,
    GTEQ,
    AMPEQ,
    BAREQ,
    DBAR,
    DQMARK,
    RARR,
    LARR,
    DLTEQ,
    DGTEQ,
    TEQ,
    BANGDEQ,
    DSLASHEQ,
    DBAREQ,
    TBAR,
    DQMARKEQ,
    TBAREQ,
};