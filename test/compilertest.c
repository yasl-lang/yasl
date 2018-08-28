#include "opcode.h"
#include "yats.h"
#include "compilertest.h"
#include "color.h"
#include "binoptest.h"
#include "unoptest.h"
#include "literaltest.h"

#define RUN(test) __YASL_TESTS_FAILED__ |= test()

SETUP_YATS();

// NOTE: these tests depend on the endianess of the system, so they may fail on big endian systems.

/// Literals
////////////////////////////////////////////////////////////////////////////////

/// Control Flow
////////////////////////////////////////////////////////////////////////////////

static void test_if() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            BRF_8,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"if true { true; };");
}

static void test_ifelse() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            BRF_8,
            0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            POP,
            BR_8,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_F,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"if true { true; } else { false; };");
}

static void test_ifelseelseif() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            BRF_8,
            0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            POP,
            BR_8,
            0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_F,
            BRF_8,
            0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_F,
            POP,
            BR_8,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            NCONST,
            POP,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"if true { true; } elseif false { false; } else { undef; };");
}

static void test_while() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BR_8,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            BRF_8,
            0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            BCONST_T,
            POP,
            BR_8,
            0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            HALT
    };
    ASSERT_GEN_BC_EQ(expected,"while true { true; };");
}

////////////////////////////////////////////////////////////////////////////////

int compilertest() {
    RUN(test_literal);
    RUN(test_unop);
    RUN(test_binop);
    // Control Flow
    test_if();
    test_ifelse();
    test_ifelseelseif();
    test_while();

    return __YASL_TESTS_FAILED__;
}