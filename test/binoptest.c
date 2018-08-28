#include "binoptest.h"
#include "yats.h"

SETUP_YATS();

static void test_mul() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            MUL,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "2 * 3;");
}

static void test_fdiv() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            FDIV,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "3 / 2;");
}

static void test_idiv() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            IDIV,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "3 // 2;");
}

static void test_mod() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            MOD,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "5 % 3;");
}

static void test_add() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ADD,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "5 + 5;");
}

static void test_sub() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            SUB,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "5 - 3;");
}

static void test_bshl() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BSL,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "2 << 3;");
}

static void test_bshr() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BSR,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"8 >> 2;");
}

static void test_band() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BAND,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"8 & 2;");
}

static void test_bxor() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BXOR,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"8 ^ 2;");
}

static void test_bor() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BOR,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"8 | 2;");
}

static void test_concat() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            CNCT,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"2 ~ 1;");
}

static void test_and() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            DUP,
            BRF_8,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            POP,
            BCONST_F,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"true && false;");
}

static void test_or() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            DUP,
            BRT_8,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            POP,
            BCONST_F,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"true || false;");
}

int test_binop(void) {
    test_mul();
    test_fdiv();
    test_idiv();
    test_mod();
    test_add();
    test_sub();
    test_bshl();
    test_bshr();
    test_band();
    test_bxor();
    test_bor();
    test_concat();
    test_and();
    test_or();

    return __YASL_TESTS_FAILED__;
}