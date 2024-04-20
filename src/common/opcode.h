#ifndef YASL_OPCODE_H_
#define YASL_OPCODE_H_

enum Opcode {
	O_NCONST = 0x01, // (byte target) set target to `undef`.
	O_BCONST_F = 0x08, // (byte target) set target to `false`.
	O_BCONST_T = 0x09, // (byte target) set target to `true`.
	O_FCONST = 0x0A, // (byte target) set target to fn.
	O_CCONST = 0x0B, // (byte target) set target to fn.

	O_HALT = 0x0F, // halt

	O_MATCH = 0x31, // pattern matching

	O_BOR = 0x40, // (byte target, byte a, byte b) set target to `a | b`.
	O_BXOR = 0x41, // (byte target, byte a, byte b) set target to `a ^ b`.
	O_BAND = 0x42, // (byte target, byte a, byte b) set target to `a & b`.
	O_BANDNOT = 0x43, // (byte target, byte a, byte b) set target to `a &^ b`.
	O_BNOT = 0x44, // (byte target, byte source) set target to `^source`.
	O_BSL = 0x45, // (byte target, byte a, byte b) set target to `a << b`.
	O_BSR = 0x46, // (byte target, byte a, byte b) set target to `a >> b`.

	O_ASS = 0x50, // assert
	O_ASSERT_STACK_HEIGHT = 0x58,

	O_ADD = 0x60, // (byte target, byte a, byte b) set target to `a + b`.
	O_SUB = 0x61, // (byte target, byte a, byte b) set target to `a - b`.
	O_MUL = 0x62, // (byte target, byte a, byte b) set target to `a * b`.
	O_EXP = 0x63, // (byte target, byte a, byte b) set target to `a ** b`.
	O_FDIV = 0x64, // (byte target, byte a, byte b) set target to `a / b`. (return float)
	O_IDIV = 0x65, // (byte target, byte a, byte b) set target to `a // b`. (return int)
	O_MOD = 0x66, // (byte target, byte a, byte b) set target to `a % b`.
	O_NEG = 0x67, // (byte target, byte source) set target to `-source`.
	O_POS = 0x68, // (byte target, byte source) set target to `+source`.
	O_NOT = 0x69, // (byte target, byte source) set target to `!source`.
	O_LEN = 0x6A, // (byte target, byte source) set target to `len source`.
	O_CNCT = 0x6B, // (byte target, byte a, byte b) set target to `a ~ b`.

	O_LT = 0x70, // (byte target, byte a, byte b) set target to `a < b`.
	O_LE = 0x71, // (byte target, byte a, byte b) set target to `a <= b`.
	O_GT = 0x72, // (byte target, byte a, byte b) set target to `a > b`.
	O_GE = 0x73, // (byte target, byte a, byte b) set target to `a >= b`.
	O_EQ = 0x74, // (byte target, byte a, byte b) set target to `a == b`.
	O_ID = 0x76, // (byte target, byte a, byte b) set target to `a === b`.

	O_SET = 0x80, // (byte target, byte a, byte b, byte c) set target to `a[b] = c`.
	O_GET = 0x88, // (byte target, byte a, byte b, byte c) set target to `a[b]`.
	O_SLICE = 0x8A, // (byte target, byte a, byte b, byte c) set target to `a[b:c]`.

	O_EXPORT = 0x90, // export

	O_LIT = 0x9A, // (byte target, byte index) sets target to the constant at index.
	O_LIT8 = 0x9B, // (byte target, i64 index) sets target to the constant at index.
	O_NEWTABLE = 0x9C, // (byte target) sets target to a new table.
	O_NEWLIST = 0x9D, // (byte target) sets target to a new list.
	O_LIST_PUSH = 0x9E,
	O_TABLE_SET = 0x9F,

	O_MOVEUP_FP = 0xA0, // move an element from index whatever to top of stack, indexing from fp.
	O_MOVEDOWN_FP = 0xA1,

	O_END = 0xB0, // indicate end of list or table on stack.
	O_SWAP = 0xB7, // swap the top two elements of the stack.
	O_DUP = 0xB8, // duplicate top values of stack
	O_DEL_FP = 0xB9, // deletes an element from the stack. Takes a one-byte index, indicating which element to delete.
	O_DECSP = 0xBD,
	O_INCSP = 0xBE,
	O_POP = 0xBF, // pop top of stack

	O_BR_8 = 0xC0, // branch unconditionally (takes next 8 bytes as jump length)
	O_BRF_8 = 0xC1, // branch if condition is falsey (takes next 8 bytes as jump length)
	O_BRT_8 = 0xC2, // branch if condition is truthy (takes next 8 bytes as jump length)
	O_BRN_8 = 0xC3, // branch if condition is not undef (takes next 8 bytes as jump length)

	O_INITFOR = 0xD0, // initialises for-loop in VM
	O_ENDCOMP = 0xD1, // end list / table comprehension
	O_ENDFOR = 0xD2, // end for-loop in VM
	O_ITER_1 = 0xD3, // iterate to next, 1 var

	O_COLLECT_REST_PARAMS = 0xE0,
	O_SPREAD_VARGS = 0xE1,
	O_INIT_MC = 0xE7,
	O_INIT_CALL = 0xE8, // set up function call
	O_CALL = 0xE9, // function call
	O_RET = 0xEC,  // return from function
	O_CRET = 0xED, // return from closure.

	O_GSTORE_8 = 0xF0, // from string
	O_GLOAD_8 = 0xF1, // from string
	O_USTORE = 0xF2, // load upvalue
	O_ULOAD = 0xF3, // store upvalue
	O_LSTORE = 0xF4, // store top of stack as local at addr
	O_LLOAD = 0xF5, // load local from addr
	O_ECHO = 0xFF  // print
};

enum Constants {
	C_FLOAT,
	C_INT_1,
	C_INT_8,
	C_STR,
};

enum Pattern {
	P_UNDEF = 0x01,
	P_TYPE_BOOL = 0x02,
	P_TYPE_INT = 0x03,
	P_TYPE_FLOAT = 0x04,
	P_TYPE_STR = 0x05,
	P_TYPE_LS = 0x06,
	P_TYPE_TABLE = 0x07,
	P_BOOL = 0x08,
	P_ANY = 0x0F,
	P_NOT = 0x10,
	P_LIT = 0x9A,
	P_LIT8 = 0x9B,
	P_TABLE = 0x9C,
	P_LS = 0x9D,
	P_VTABLE = 0xAC,
	P_VLS = 0xAD,
	P_ALT = 0xB0,
	P_BIND = 0xF4,
};

#endif
