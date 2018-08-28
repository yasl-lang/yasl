#include "unoptest.h"
#include "yats.h"

SETUP_YATS();

static void test_neg() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            NEG,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "-16;");
}

static void test_len() {
    unsigned char expected[] = {
            0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            'Y', 'A', 'S', 'L',
            NEWSTR,
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            LEN,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "@'YASL';");
}

static void test_not() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            NOT,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "!true;");
}

static void test_bnot() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            ICONST,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BNOT,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected, "^0x00;");
}

int test_unop(void) {
    test_len();
    test_neg();
    test_not();
    test_bnot();

    return __YASL_TESTS_FAILED__;
}