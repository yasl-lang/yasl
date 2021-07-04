#include "compiler/lexer.h"
#include "test/yats.h"

SETUP_YATS();

#define ASSERT_TOK_EQ(left, right) do {\
    if (left == right) {\
        /*printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);*/\
    } else {\
        printf(K_RED "assert failed in %s (in %s): line %d: `%s` =/= `%s`" K_END "\n", __FILE__, __func__, __LINE__, YASL_TOKEN_NAMES[left], YASL_TOKEN_NAMES[right]);\
        TEST_FAILED();\
    }\
} while(0)

#define ASSERT_EATTOK(tok, lex) do {\
            gettok(&(lex));\
            ASSERT_TOK_EQ(tok, (lex).type);\
	    free((lex).buffer.bytes);\
        } while(0)

#define USING_LEX(name, val, ...) do {\
	struct Lexer name = setup_lexer(val);\
	__VA_ARGS__\
	lex_cleanup(&name);\
} while (0)

static void test_semi(void) {
	USING_LEX(lex, ";",
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_assert(void) {
	USING_LEX(lex, "assert",
		ASSERT_EATTOK(T_ASS, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_undef(void) {
	USING_LEX(lex, "undef",
		ASSERT_EATTOK(T_UNDEF, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_valid_float(void) {
	USING_LEX(lex, "6.4",
		ASSERT_EATTOK(T_FLOAT, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_invalid_float_no_leading_digit(void) {
	USING_LEX(lex, ".4",
		ASSERT_EATTOK(T_DOT, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_invalid_float_no_trailing_digit(void) {
	USING_LEX(lex, "4.",
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_DOT, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_bool_false(void) {
	USING_LEX(lex, "false",
		ASSERT_EATTOK(T_BOOL, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_bool_true(void) {
	USING_LEX(lex, "true",
		ASSERT_EATTOK(T_BOOL, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_if(void) {
	USING_LEX(lex, "if",
		ASSERT_EATTOK(T_IF, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_elseif(void) {
	USING_LEX(lex, "elseif",
		ASSERT_EATTOK(T_ELSEIF, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_else(void) {
	USING_LEX(lex, "else",
		ASSERT_EATTOK(T_ELSE, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_while(void) {
	USING_LEX(lex, "while",
		ASSERT_EATTOK(T_WHILE, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_break(void) {
	USING_LEX(lex, "break",
		ASSERT_EATTOK(T_BREAK, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_continue(void) {
	USING_LEX(lex, "continue",
		ASSERT_EATTOK(T_CONT, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_for(void) {
	USING_LEX(lex, "for",
		ASSERT_EATTOK(T_FOR, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_const(void) {
	USING_LEX(lex, "const",
		ASSERT_EATTOK(T_CONST, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_fn(void) {
	USING_LEX(lex, "fn",
		ASSERT_EATTOK(T_FN, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_return(void) {
	USING_LEX(lex, "return",
	ASSERT_EATTOK(T_RET, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_export(void) {
	USING_LEX(lex, "export",
	ASSERT_EATTOK(T_EXPORT, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dec(void) {
	USING_LEX(lex, "echo",
	ASSERT_EATTOK(T_ECHO, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_lpar(void) {
	USING_LEX(lex, "(",
	ASSERT_EATTOK(T_LPAR, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_rpar(void) {
	USING_LEX(lex, ")",
	ASSERT_EATTOK(T_RPAR, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_lsqb(void) {
	USING_LEX(lex, "[",
	ASSERT_EATTOK(T_LSQB, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_rsqb(void) {
	USING_LEX(lex, "]",
	ASSERT_EATTOK(T_RSQB, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_lbrc(void) {
	USING_LEX(lex, "{",
	ASSERT_EATTOK(T_LBRC, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_rbrc(void) {
	USING_LEX(lex, "}",
	ASSERT_EATTOK(T_RBRC, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dot(void) {
	USING_LEX(lex, ".",
	ASSERT_EATTOK(T_DOT, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_comma(void) {
	USING_LEX(lex, ",",
	ASSERT_EATTOK(T_COMMA, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_caret(void) {
	USING_LEX(lex, "^",
	ASSERT_EATTOK(T_CARET, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_careteq(void) {
	USING_LEX(lex, "^=",
	ASSERT_EATTOK(T_CARETEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_plus(void) {
	USING_LEX(lex, "+",
	ASSERT_EATTOK(T_PLUS, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_pluseq(void) {
	USING_LEX(lex, "+=",
	ASSERT_EATTOK(T_PLUSEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_minus(void) {
	USING_LEX(lex, "-",
	ASSERT_EATTOK(T_MINUS, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_minuseq(void) {
	USING_LEX(lex, "-=",
	ASSERT_EATTOK(T_MINUSEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_at(void) {
	USING_LEX(lex, "len",
	ASSERT_EATTOK(T_LEN, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_bang(void) {
	USING_LEX(lex, "!",
	ASSERT_EATTOK(T_BANG, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_bangeq(void) {
	USING_LEX(lex, "!=",
	ASSERT_EATTOK(T_BANGEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_bangdeq(void) {
	USING_LEX(lex, "!==",
	ASSERT_EATTOK(T_BANGDEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_tilde(void) {
	USING_LEX(lex, "~",
	ASSERT_EATTOK(T_TILDE, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_tildeeq(void) {
	USING_LEX(lex, "~=",
	ASSERT_EATTOK(T_TILDEEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_star(void) {
	USING_LEX(lex, "*",
	ASSERT_EATTOK(T_STAR, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_stareq(void) {
	USING_LEX(lex, "*=",
	ASSERT_EATTOK(T_STAREQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dstar(void) {
	USING_LEX(lex, "**",
	ASSERT_EATTOK(T_DSTAR, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dstareq(void) {
	USING_LEX(lex, "**=",
	ASSERT_EATTOK(T_DSTAREQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_slash(void) {
	USING_LEX(lex, "/",
	ASSERT_EATTOK(T_SLASH, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_slasheq(void) {
	USING_LEX(lex, "/=",
	ASSERT_EATTOK(T_SLASHEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dslash(void) {
	USING_LEX(lex, "//",
	ASSERT_EATTOK(T_DSLASH, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dslasheq(void) {
	USING_LEX(lex, "//=",
	ASSERT_EATTOK(T_DSLASHEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_mod(void) {
	USING_LEX(lex, "%",
	ASSERT_EATTOK(T_MOD, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_modeq(void) {
	USING_LEX(lex, "%=",
	ASSERT_EATTOK(T_MODEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_lt(void) {
	USING_LEX(lex, "<",
	ASSERT_EATTOK(T_LT, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_lteq(void) {
	USING_LEX(lex, "<=",
	ASSERT_EATTOK(T_LTEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dlt(void) {
	USING_LEX(lex, "<<",
	ASSERT_EATTOK(T_DLT, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dlteq(void) {
	USING_LEX(lex, "<<=",
	ASSERT_EATTOK(T_DLTEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_gt(void) {
	USING_LEX(lex, ">",
	ASSERT_EATTOK(T_GT, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_gteq(void) {
	USING_LEX(lex, ">=",
	ASSERT_EATTOK(T_GTEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dgt(void) {
	USING_LEX(lex, ">>",
	ASSERT_EATTOK(T_DGT, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dgteq(void) {
	USING_LEX(lex, ">>=",
	ASSERT_EATTOK(T_DGTEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_eq(void) {
	USING_LEX(lex, "=",
	ASSERT_EATTOK(T_EQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_deq(void) {
	USING_LEX(lex, "==",
	ASSERT_EATTOK(T_DEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_teq(void) {
	USING_LEX(lex, "===",
	ASSERT_EATTOK(T_TEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_amp(void) {
	USING_LEX(lex, "&",
	ASSERT_EATTOK(T_AMP, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_ampeq(void) {
	USING_LEX(lex, "&=",
	ASSERT_EATTOK(T_AMPEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_ampcaret(void) {
	USING_LEX(lex, "&^",
	ASSERT_EATTOK(T_AMPCARET, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_ampcareteq(void) {
	USING_LEX(lex, "&^=",
	ASSERT_EATTOK(T_AMPCARETEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_damp(void) {
	USING_LEX(lex, "&&",
	ASSERT_EATTOK(T_DAMP, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dampeq(void) {
	USING_LEX(lex, "&&=",
	ASSERT_EATTOK(T_DAMPEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_bar(void) {
	USING_LEX(lex, "|",
	ASSERT_EATTOK(T_BAR, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_bareq(void) {
	USING_LEX(lex, "|=",
	ASSERT_EATTOK(T_BAREQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dbar(void) {
	USING_LEX(lex, "||",
	ASSERT_EATTOK(T_DBAR, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dbareq(void) {
	USING_LEX(lex, "||=",
	ASSERT_EATTOK(T_DBAREQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_qmark(void) {
	USING_LEX(lex, "?",
	ASSERT_EATTOK(T_QMARK, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dqmark(void) {
	USING_LEX(lex, "??",
	ASSERT_EATTOK(T_DQMARK, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_dqmarkeq(void) {
	USING_LEX(lex, "?\?=",
	ASSERT_EATTOK(T_DQMARKEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_colon(void) {
	USING_LEX(lex, ":",
	ASSERT_EATTOK(T_COLON, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_colon_eq(void) {
	USING_LEX(lex, ":=",
	ASSERT_EATTOK(T_COLONEQ, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_small_arrow(void) {
	USING_LEX(lex, "->",
	ASSERT_EATTOK(T_RIGHT_ARR, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_int(void) {
	USING_LEX(lex, "64;"
		       "0__10;"
		       "1_000;"
		       "10__;"
		       "1_000__000___;",
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_hex(void) {
	USING_LEX(lex, "0x10;"
		       "0x__10;"
		       "0x10_AB;"
		       "0x10__;"
		       "0x_10__AB_;",
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_binary(void) {
	USING_LEX(lex, "0b10;"
		       "0b__10;"
		       "0b10_10;"
		       "0b10__;"
		       "0b_10__10_;",
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_INT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_float(void) {
	USING_LEX(lex, "64.50;"
		       "1_0___.5__;"
		       "1____.6_7__8;"
		       "1.5_;",
		ASSERT_EATTOK(T_FLOAT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_FLOAT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_FLOAT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_FLOAT, lex);
		ASSERT_EATTOK(T_SEMI, lex);
		ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_string(void) {
	USING_LEX(lex, "x := 'hello world';",
	ASSERT_EATTOK(T_ID, lex);
	ASSERT_EATTOK(T_COLONEQ, lex);
	ASSERT_EATTOK(T_STR, lex);
	ASSERT_EATTOK(T_SEMI, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

static void test_division(void) {
	USING_LEX(lex, "5 / 7.0",
	ASSERT_EATTOK(T_INT, lex);
	ASSERT_EATTOK(T_SLASH, lex);
	ASSERT_EATTOK(T_FLOAT, lex);
	ASSERT_EATTOK(T_EOF, lex);
	);
}

int lexertest(void) {
	test_int();
	test_hex();
	test_binary();
	test_float();
	test_string();
	test_division();

	test_semi();
	test_assert();
	test_undef();
	test_valid_float();
	test_invalid_float_no_leading_digit();
	test_invalid_float_no_trailing_digit();
	test_bool_false();
	test_bool_true();
	test_if();
	test_elseif();
	test_else();
	test_while();
	test_break();
	test_continue();
	test_for();
	test_const();
	test_fn();
	test_return();
	test_export();
	test_dec();
	test_lpar();
	test_rpar();
	test_lsqb();
	test_rsqb();
	test_lbrc();
	test_rbrc();
	test_dot();
	test_comma();
	test_caret();
	test_careteq();
	test_plus();
	test_pluseq();
	test_minus();
	test_minuseq();
	test_at();
	test_bang();
	test_bangeq();
	test_bangdeq();
	test_tilde();
	test_tildeeq();
	test_star();
	test_stareq();
	test_dstar();
	test_dstareq();
	test_slash();
	test_slasheq();
	test_dslash();
	test_dslasheq();
	test_mod();
	test_modeq();
	test_lt();
	test_lteq();
	test_dlt();
	test_dlteq();
	test_gt();
	test_gteq();
	test_dgt();
	test_dgteq();
	test_eq();
	test_deq();
	test_teq();
	test_amp();
	test_ampeq();
	test_ampcaret();
	test_ampcareteq();
	test_damp();
	test_dampeq();
	test_bar();
	test_bareq();
	test_dbar();
	test_dbareq();
	test_qmark();
	test_dqmark();
	test_dqmarkeq();
	test_colon();
	test_colon_eq();
	test_small_arrow();
	return NUM_FAILED;
}
