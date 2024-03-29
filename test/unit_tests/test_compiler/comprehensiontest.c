#include "comprehensiontest.h"
#include "yats.h"

SETUP_YATS();

static void test_tablecomp_noif() {
	unsigned char expected[] = {
		0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 1,
		C_INT_1, 2,
		C_INT_1, 3,
		O_END,
		O_LIT, 0x00,
		O_LIT, 0x01,
		O_LIT, 0x02,
		O_NEWLIST,
		O_INITFOR,
		O_END,
		O_NEWTABLE,
		O_END,
		O_MOVEDOWN_FP, 0X00,
		O_ITER_1,
		O_BRF_8,
		0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LSTORE, 0x00,
		O_LLOAD, 0x00,
		O_LLOAD, 0x00,
		O_NEG,
		O_TABLE_SET,
		O_BR_8,
		0xE5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_ENDCOMP, 0X00,
		O_ECHO, 0x00,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo {i:-i for i in [1,2,3]};");
}

static void test_tablecomp() {
	unsigned char expected[] = {
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 1,
		C_INT_1, 2,
		C_INT_1, 3,
		C_INT_1, 0,
		O_END,
		O_LIT, 0x00,
		O_LIT, 0x01,
		O_LIT, 0x02,
		O_NEWLIST,
		O_INITFOR,
		O_END,
		O_NEWTABLE,
		O_END,
		O_MOVEDOWN_FP, 0X00,
		O_ITER_1,
		O_BRF_8,
		0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LSTORE, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x01,
		O_MOD,
		O_LIT, 0x03,
		O_EQ,
		O_NOT,
		O_BRF_8,
		0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LLOAD, 0x00,
		O_LLOAD, 0x00,
		O_NEG,
		O_TABLE_SET,
		O_BR_8,
		0xD3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_ENDCOMP, 0X00,
		O_ECHO, 0x00,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo {i:-i for i in [1,2,3] if i % 2 != 0};");
}

static void test_listcomp_noif() {
	unsigned char expected[] = {
		0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 1,
		C_INT_1, 2,
		C_INT_1, 3,
		O_END,
		O_LIT, 0x00,
		O_LIT, 0x01,
		O_LIT, 0x02,
		O_NEWLIST,
		O_INITFOR,
		O_END,
		O_NEWLIST,
		O_END,
		O_MOVEDOWN_FP, 0X00,
		O_ITER_1,
		O_BRF_8,
		0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LSTORE, 0x00,
		O_LLOAD, 0x00,
		O_NEG,
		O_LIST_PUSH,
		O_BR_8,
		0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_ENDCOMP, 0X00,
		O_ECHO, 0x00,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo [-i for i in [1,2,3]];");
}

static void test_listcomp() {
	unsigned char expected[] = {
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 1,
		C_INT_1, 2,
		C_INT_1, 3,
		C_INT_1, 0,
		O_END,
		O_LIT, 0x00,
		O_LIT, 0x01,
		O_LIT, 0x02,
		O_NEWLIST,
		O_INITFOR,
		O_END,
		O_NEWLIST,
		O_END,
		O_MOVEDOWN_FP, 0X00,
		O_ITER_1,
		O_BRF_8,
		0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LSTORE, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x01,
		O_MOD,
		O_LIT, 0x03,
		O_EQ,
		O_NOT,
		O_BRF_8,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LLOAD, 0x00,
		O_NEG,
		O_LIST_PUSH,
		O_BR_8,
		0xD5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_ENDCOMP, 0X00,
		O_ECHO, 0x00,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo [-i for i in [1,2,3] if i % 2 != 0];");
}

TEST(comprehensiontest) {
	test_listcomp();
	test_listcomp_noif();
	test_tablecomp();
	test_tablecomp_noif();
	return NUM_FAILED;
}
