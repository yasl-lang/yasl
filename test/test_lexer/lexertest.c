#include "lexer.h"
#include "test/test_compiler/yats.h"

SETUP_YATS();

#define ASSERT_TOK_EQ(left, right) do {\
    if (left == right) {\
        /*printf(K_GRN "assert passed in %s: line %d" K_END "\n", __func__, __LINE__);*/\
    } else {\
        printf(K_RED "assert failed in %s (in %s): line %d: `%s` =/= `%s`" K_END "\n", __FILE__, __func__, __LINE__, YASL_TOKEN_NAMES[left], YASL_TOKEN_NAMES[right]);\
        __YASL_TESTS_FAILED__ = 1;\
    }\
} while(0)

#define ASSERT_EATTOK(tok, lex) do {\
            gettok(&(lex));\
            ASSERT_TOK_EQ(tok, (lex).type);\
        } while(0)

void test_semi(void) {
    Lexer lex = setup_lexer(";");
    ASSERT_EATTOK(T_SEMI, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_undef(void) {
    Lexer lex = setup_lexer("undef");
    ASSERT_EATTOK(T_UNDEF, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_valid_float(void) {
    Lexer lex = setup_lexer("6.4");
    ASSERT_EATTOK(T_FLOAT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_invalid_float_no_leading_digit(void) {
    Lexer lex = setup_lexer(".4");
    ASSERT_EATTOK(T_DOT, lex);
    ASSERT_EATTOK(T_INT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_invalid_float_no_trailing_digit(void) {
    Lexer lex = setup_lexer("4.");
    ASSERT_EATTOK(T_INT, lex);
    ASSERT_EATTOK(T_DOT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_bool_false(void) {
    Lexer lex = setup_lexer("false");
    ASSERT_EATTOK(T_BOOL, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_bool_true(void) {
    Lexer lex = setup_lexer("true");
    ASSERT_EATTOK(T_BOOL, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_if(void) {
    Lexer lex = setup_lexer("if");
    ASSERT_EATTOK(T_IF, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_elseif(void) {
    Lexer lex = setup_lexer("elseif");
    ASSERT_EATTOK(T_ELSEIF, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_else(void) {
    Lexer lex = setup_lexer("else");
    ASSERT_EATTOK(T_ELSE, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_while(void) {
    Lexer lex = setup_lexer("while");
    ASSERT_EATTOK(T_WHILE, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_break(void) {
    Lexer lex = setup_lexer("break");
    ASSERT_EATTOK(T_BREAK, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_continue(void) {
    Lexer lex = setup_lexer("continue");
    ASSERT_EATTOK(T_CONT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_for(void) {
    Lexer lex = setup_lexer("for");
    ASSERT_EATTOK(T_FOR, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_in(void) {
    Lexer lex = setup_lexer("in");
    ASSERT_EATTOK(T_IN, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_not_in(void) {
	Lexer lex = setup_lexer("!in");
	ASSERT_EATTOK(T_BANG, lex);
	ASSERT_EATTOK(T_IN, lex);
	ASSERT_EATTOK(T_EOF, lex);
}

void test_const(void) {
    Lexer lex = setup_lexer("const");
    ASSERT_EATTOK(T_CONST, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_fn(void) {
    Lexer lex = setup_lexer("fn");
    ASSERT_EATTOK(T_FN, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_return(void) {
    Lexer lex = setup_lexer("return");
    ASSERT_EATTOK(T_RET, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_enum(void) {
    Lexer lex = setup_lexer("enum");
    ASSERT_EATTOK(T_ENUM, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dec(void) {
    Lexer lex = setup_lexer("echo");
    ASSERT_EATTOK(T_ECHO, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_lpar(void) {
    Lexer lex = setup_lexer("(");
    ASSERT_EATTOK(T_LPAR, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_rpar(void) {
    Lexer lex = setup_lexer(")");
    ASSERT_EATTOK(T_RPAR, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_lsqb(void) {
    Lexer lex = setup_lexer("[");
    ASSERT_EATTOK(T_LSQB, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_rsqb(void) {
    Lexer lex = setup_lexer("]");
    ASSERT_EATTOK(T_RSQB, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_lbrc(void) {
    Lexer lex = setup_lexer("{");
    ASSERT_EATTOK(T_LBRC, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_rbrc(void) {
    Lexer lex = setup_lexer("}");
    ASSERT_EATTOK(T_RBRC, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dot(void) {
    Lexer lex = setup_lexer(".");
    ASSERT_EATTOK(T_DOT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_comma(void) {
    Lexer lex = setup_lexer(",");
    ASSERT_EATTOK(T_COMMA, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_caret(void) {
    Lexer lex = setup_lexer("^");
    ASSERT_EATTOK(T_CARET, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_careteq(void) {
    Lexer lex = setup_lexer("^=");
    ASSERT_EATTOK(T_CARETEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_plus(void) {
    Lexer lex = setup_lexer("+");
    ASSERT_EATTOK(T_PLUS, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_pluseq(void) {
    Lexer lex = setup_lexer("+=");
    ASSERT_EATTOK(T_PLUSEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_minus(void) {
    Lexer lex = setup_lexer("-");
    ASSERT_EATTOK(T_MINUS, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_minuseq(void) {
    Lexer lex = setup_lexer("-=");
    ASSERT_EATTOK(T_MINUSEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_at(void) {
    Lexer lex = setup_lexer("len");
    ASSERT_EATTOK(T_LEN, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_bang(void) {
    Lexer lex = setup_lexer("!");
    ASSERT_EATTOK(T_BANG, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_bangeq(void) {
    Lexer lex = setup_lexer("!=");
    ASSERT_EATTOK(T_BANGEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_bangdeq(void) {
    Lexer lex = setup_lexer("!==");
    ASSERT_EATTOK(T_BANGDEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_tilde(void) {
    Lexer lex = setup_lexer("~");
    ASSERT_EATTOK(T_TILDE, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_tildeeq(void) {
    Lexer lex = setup_lexer("~=");
    ASSERT_EATTOK(T_TILDEEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_star(void) {
    Lexer lex = setup_lexer("*");
    ASSERT_EATTOK(T_STAR, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_stareq(void) {
    Lexer lex = setup_lexer("*=");
    ASSERT_EATTOK(T_STAREQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dstar(void) {
    Lexer lex = setup_lexer("**");
    ASSERT_EATTOK(T_DSTAR, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dstareq(void) {
    Lexer lex = setup_lexer("**=");
    ASSERT_EATTOK(T_DSTAREQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_slash(void) {
    Lexer lex = setup_lexer("/");
    ASSERT_EATTOK(T_SLASH, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_slasheq(void) {
    Lexer lex = setup_lexer("/=");
    ASSERT_EATTOK(T_SLASHEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dslash(void) {
    Lexer lex = setup_lexer("//");
    ASSERT_EATTOK(T_DSLASH, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dslasheq(void) {
    Lexer lex = setup_lexer("//=");
    ASSERT_EATTOK(T_DSLASHEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_mod(void) {
    Lexer lex = setup_lexer("%");
    ASSERT_EATTOK(T_MOD, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_modeq(void) {
    Lexer lex = setup_lexer("%=");
    ASSERT_EATTOK(T_MODEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_lt(void) {
    Lexer lex = setup_lexer("<");
    ASSERT_EATTOK(T_LT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_lteq(void) {
    Lexer lex = setup_lexer("<=");
    ASSERT_EATTOK(T_LTEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dlt(void) {
    Lexer lex = setup_lexer("<<");
    ASSERT_EATTOK(T_DLT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dlteq(void) {
    Lexer lex = setup_lexer("<<=");
    ASSERT_EATTOK(T_DLTEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_gt(void) {
    Lexer lex = setup_lexer(">");
    ASSERT_EATTOK(T_GT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_gteq(void) {
    Lexer lex = setup_lexer(">=");
    ASSERT_EATTOK(T_GTEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dgt(void) {
    Lexer lex = setup_lexer(">>");
    ASSERT_EATTOK(T_DGT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dgteq(void) {
    Lexer lex = setup_lexer(">>=");
    ASSERT_EATTOK(T_DGTEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_eq(void) {
    Lexer lex = setup_lexer("=");
    ASSERT_EATTOK(T_EQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_deq(void) {
    Lexer lex = setup_lexer("==");
    ASSERT_EATTOK(T_DEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_teq(void) {
    Lexer lex = setup_lexer("===");
    ASSERT_EATTOK(T_TEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_amp(void) {
    Lexer lex = setup_lexer("&");
    ASSERT_EATTOK(T_AMP, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_ampeq(void) {
    Lexer lex = setup_lexer("&=");
    ASSERT_EATTOK(T_AMPEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_ampcaret(void) {
    Lexer lex = setup_lexer("&^");
    ASSERT_EATTOK(T_AMPCARET, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_ampcareteq(void) {
    Lexer lex = setup_lexer("&^=");
    ASSERT_EATTOK(T_AMPCARETEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_damp(void) {
    Lexer lex = setup_lexer("&&");
    ASSERT_EATTOK(T_DAMP, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dampeq(void) {
    Lexer lex = setup_lexer("&&=");
    ASSERT_EATTOK(T_DAMPEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}


void test_bar(void) {
    Lexer lex = setup_lexer("|");
    ASSERT_EATTOK(T_BAR, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_bareq(void) {
    Lexer lex = setup_lexer("|=");
    ASSERT_EATTOK(T_BAREQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dbar(void) {
    Lexer lex = setup_lexer("||");
    ASSERT_EATTOK(T_DBAR, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dbareq(void) {
    Lexer lex = setup_lexer("||=");
    ASSERT_EATTOK(T_DBAREQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_qmark(void) {
    Lexer lex = setup_lexer("?");
    ASSERT_EATTOK(T_QMARK, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dqmark(void) {
    Lexer lex = setup_lexer("??");
    ASSERT_EATTOK(T_DQMARK, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_dqmarkeq(void) {
    Lexer lex = setup_lexer("?\?=");
    ASSERT_EATTOK(T_DQMARKEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_colon(void) {
    Lexer lex = setup_lexer(":");
    ASSERT_EATTOK(T_COLON, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_colon_eq(void) {
    Lexer lex = setup_lexer(":=");
    ASSERT_EATTOK(T_COLONEQ, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_small_arrow(void) {
    Lexer lex = setup_lexer("->");
    ASSERT_EATTOK(T_RIGHT_ARR, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_int(void) {
    Lexer lex = setup_lexer("64\n");
    ASSERT_EATTOK(T_INT, lex);
    ASSERT_EATTOK(T_SEMI, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_string(void) {
    Lexer lex = setup_lexer("x := 'hello world';");
    ASSERT_EATTOK(T_ID, lex);
    ASSERT_EATTOK(T_COLONEQ, lex);
    ASSERT_EATTOK(T_STR, lex);
    ASSERT_EATTOK(T_SEMI, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

void test_division(void) {
    Lexer lex = setup_lexer("5 / 7.0");
    ASSERT_EATTOK(T_INT, lex);
    ASSERT_EATTOK(T_SLASH, lex);
    ASSERT_EATTOK(T_FLOAT, lex);
    ASSERT_EATTOK(T_EOF, lex);
}

int lexertest(void) {
    test_int();
    test_string();
    test_division();

    test_semi();
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
    test_in();
    test_not_in();
    test_const();
    test_fn();
    test_return();
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
    return __YASL_TESTS_FAILED__;
}