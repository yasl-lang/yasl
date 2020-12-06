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

static int testpushundef(void) {
	unsigned char code[] = {
		O_NCONST,
	};

	struct VM vm;
	vm_init(&vm, code, 0, sizeof(code));

	ASSERT_INC(&vm);
	return __YASL_TESTS_FAILED__;
}

static int testpushbool(void) {
	unsigned char code[] = {
		O_BCONST_F,
		O_BCONST_T
	};

	struct VM vm;
	vm_init(&vm, code, 0, sizeof(code));

	ASSERT_INC(&vm);
	ASSERT_INC(&vm);
	return __YASL_TESTS_FAILED__;
}

static int testpushint(void) {
	unsigned char code[] = {
		O_ICONST_B1, 0x10
	};

	struct VM vm;
	vm_init(&vm, code, 0, sizeof(code));

	ASSERT_INC(&vm);
	return __YASL_TESTS_FAILED__;
}

static int testpushfloat(void) {
	unsigned char code[] = {
		O_DCONST, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	struct VM vm;
	vm_init(&vm, code, 0, sizeof(code));

	ASSERT_INC(&vm);
	return __YASL_TESTS_FAILED__;
}

static int testpushstr(void) {
	unsigned char code[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // First two lines are just for offset
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_SCONST,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		'Y', 'A', 'S', 'L',
		O_NEWSTR,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	struct VM vm;

	vm_init(&vm, code, 0x25, sizeof(code));
	vm_setupconstants(&vm);

	ASSERT_INC(&vm);
	return __YASL_TESTS_FAILED__;
}

static int testrmrange(void) {
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


	return __YASL_TESTS_FAILED__;
}

static int testrmrangetop(void) {
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


	return __YASL_TESTS_FAILED__;
}

static int testrmrangetotop(void) {
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


	return __YASL_TESTS_FAILED__;
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

	return __YASL_TESTS_FAILED__;
}
