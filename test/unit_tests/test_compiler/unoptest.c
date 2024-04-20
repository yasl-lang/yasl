#include "unoptest.h"
#include "yats.h"

SETUP_YATS();

static void test_neg() {
	unsigned char expected[] = {
		0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 0x10,
		O_LIT, 0x00,
		O_LLOAD, 0x00,
		O_NEG, 0x01, 0x01,
		O_POP,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "let x = 16; -x;");
}

static void test_len() {
	unsigned char expected[] = {
		0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x2E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_STR,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		'Y', 'A', 'S', 'L',
		O_LIT, 0x00,
		O_LLOAD, 0x00,
		O_LEN, 0x01, 0x01,
		O_POP,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "let x = 'YASL'; len x;");
}

static void test_not() {
	unsigned char expected[] = {
		0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_BCONST_T,
		O_LLOAD, 0x00,
		O_NOT, 0x01, 0x01,
		O_POP,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "let x = true; !x;");
}

static void test_bnot() {
	unsigned char expected[] = {
		0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 0,
		O_LIT, 0x00,
		O_LLOAD, 0x00,
		O_BNOT, 0x01, 0x01,
		O_POP,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "let x = 0x00; ^x;");
}

TEST(unoptest) {
	test_len();
	test_neg();
	test_not();
	test_bnot();

	return NUM_FAILED;
}
