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


// NOTE: make sure that augmented version of operand is directly after regular version for all operands.
// parser.c uses this
// NOTE: keep up to date with array names in lexer.c
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
	T_IN,         // in
	T_BANGIN,     // !in
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
	T_HASH,       // #
	T_AT,         // @
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
