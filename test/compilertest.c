#include "opcode.h"
#include "yats.h"
#include "compilertest.h"
#include "color.h"
#include "binoptest.h"
#include "unoptest.h"
#include "literaltest.h"
#include "iftest.h"

#define RUN(test) __YASL_TESTS_FAILED__ |= test()

SETUP_YATS();

// NOTE: these tests depend on the endianess of the system, so they may fail on big endian systems.

/// Control Flow
////////////////////////////////////////////////////////////////////////////////

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
    RUN(literaltest);
    RUN(unoptest);
    RUN(binoptest);
    RUN(iftest);
    // Control Flow
    test_while();

    return __YASL_TESTS_FAILED__;
}