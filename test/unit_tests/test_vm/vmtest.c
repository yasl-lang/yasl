#include "test/yats.h"
#include "yasl.h"
#include "yasl_aux.h"
#include "IO.h"
#include "yasl_state.h"

SETUP_YATS();

////////////////////////////////////////////////////////////////////////////////

void vm_setupconstants(struct VM *const vm);
void vm_executenext(struct VM *const vm);
void vm_rm_range(struct VM *const vm, int start, int end);

#define ASSERT_INC(vm) do {\
int before = (vm)->sp;\
vm_executenext(vm);\
int after = (vm)->sp;\
ASSERT_EQ(before + 1, after);\
} while (0)

static int TEST(testpushundef) {
	unsigned char code[] = {
		O_NCONST,
	};

	struct VM vm;
	vm_init(&vm, code, 0, sizeof(code));

	ASSERT_INC(&vm);
	return NUM_FAILED;
}

static int TEST(testpushbool) {
	unsigned char code[] = {
		O_BCONST_F,
		O_BCONST_T
	};

	struct VM vm;
	vm_init(&vm, code, 0, sizeof(code));

	ASSERT_INC(&vm);
	ASSERT_INC(&vm);
	return NUM_FAILED;
}

static int TEST(testpushint) {
	unsigned char code[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // First two lines are just for offset
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 0x10,
		O_LIT, 0x00
	};

	struct VM vm;

	vm_init(&vm, code, 0x1A, sizeof(code));
	vm_setupconstants(&vm);

	ASSERT_INC(&vm);
	return NUM_FAILED;
}

static int TEST(testpushfloat) {
	unsigned char code[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // First two lines are just for offset
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_FLOAT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LIT, 0x00
	};

	struct VM vm;

	vm_init(&vm, code, 0x21, sizeof(code));
	vm_setupconstants(&vm);

	ASSERT_INC(&vm);
	return NUM_FAILED;
}

static int TEST(testpushstr) {
	unsigned char code[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // First two lines are just for offset
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_STR,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		'Y', 'A', 'S', 'L',
		O_LIT, 0x00
	};

	struct VM vm;

	vm_init(&vm, code, 0x25, sizeof(code));
	vm_setupconstants(&vm);

	ASSERT_INC(&vm);
	return NUM_FAILED;
}

static int TEST(testrmrange) {
	struct VM vm;
	vm_init(&vm, NULL, 0, 0);

	for (int i = 0; i < 10; i++) {
		vm_pushint(&vm, i);
	}

	vm_rm_range(&vm, 4, 7);

	ASSERT_EQ(vm_popint(&vm), 9);
	ASSERT_EQ(vm_popint(&vm), 8);
	ASSERT_EQ(vm_popint(&vm), 7);
	ASSERT_EQ(vm_popint(&vm), 3);
	ASSERT_EQ(vm_popint(&vm), 2);
	ASSERT_EQ(vm_popint(&vm), 1);
	ASSERT_EQ(vm_popint(&vm), 0);


	return NUM_FAILED;
}

static int TEST(testrmrangetop) {
	struct VM vm;
	vm_init(&vm, NULL, 0, 0);

	for (int i = 0; i < 10; i++) {
		vm_pushint(&vm, i);
	}

	vm_rm_range(&vm, 4, 9);

	ASSERT_EQ(vm_popint(&vm), 9);
	ASSERT_EQ(vm_popint(&vm), 3);
	ASSERT_EQ(vm_popint(&vm), 2);
	ASSERT_EQ(vm_popint(&vm), 1);
	ASSERT_EQ(vm_popint(&vm), 0);


	return NUM_FAILED;
}

static int TEST(testrmrangetotop) {
	struct VM vm;
	vm_init(&vm, NULL, 0, 0);

	for (int i = 0; i < 10; i++) {
		vm_pushint(&vm, i);
	}

	vm_rm_range(&vm, 4, 10);

	ASSERT_EQ(vm_popint(&vm), 3);
	ASSERT_EQ(vm_popint(&vm), 2);
	ASSERT_EQ(vm_popint(&vm), 1);
	ASSERT_EQ(vm_popint(&vm), 0);


	return NUM_FAILED;
}

int vmtest(void) {
	RUN(testpushundef);
	RUN(testpushbool);
	RUN(testpushint);
	RUN(testpushfloat);
	RUN(testpushstr);
	RUN(testrmrange);
	RUN(testrmrangetop);
	RUN(testrmrangetotop);

	return NUM_FAILED;
}
