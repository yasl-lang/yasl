#include "fortest.h"
#include "yats.h"

SETUP_YATS();

/*
static void test_while() {
    unsigned char expected[] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
*/

static void test_continue() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x5B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		ICONST_0,
		GSTORE_1, 0x00,
		BR_8,
		0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		GLOAD_1, 0x00,
		ICONST_1,
		ADD,
		GSTORE_1, 0x00,
		GLOAD_1, 0x00,
		ICONST,
		0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		GE,
		NOT,
		BRF_8,
		0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		GLOAD_1, 0x00,
		ICONST_5,
		EQ,
		BRF_8,
		0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BR_8,
		0xCE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		GLOAD_1, 0x00,
		PRINT,
		BR_8,
		0xC2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		HALT
	};
	ASSERT_GEN_BC_EQ(expected, "for let i = 0; i < 10; i += 1 { if i == 5 { continue; }; echo i; };");
}

static void test_break() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		ICONST_0,
		GSTORE_1, 0x00,
		BR_8,
		0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		GLOAD_1, 0x00,
		ICONST_1,
		ADD,
		GSTORE_1, 0x00,
		GLOAD_1, 0x00,
		ICONST,
		0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		GE,
		NOT,
		BRF_8,
		0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		GLOAD_1, 0x00,
		ICONST_5,
		EQ,
		BRF_8,
		0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_F,
		BR_8,
		0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		GLOAD_1, 0x00,
		PRINT,
		BR_8,
		0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		HALT
	};
	ASSERT_GEN_BC_EQ(expected, "for let i = 0; i < 10; i += 1 { if i == 5 { break; }; echo i; };");
}

int fortest(void) {
	// test_for();
	test_continue();
	test_break();
	return __YASL_TESTS_FAILED__;
}