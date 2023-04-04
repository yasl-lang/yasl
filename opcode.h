#ifndef YASL_OPCODE_H_
#define YASL_OPCODE_H_

enum Opcode {
	O_NCONST = 0x01, // push literal undef onto stack
	O_BCONST_F = 0x08, // push literal false onto stack
	O_BCONST_T = 0x09, // push literal true onto stack
	O_FCONST = 0x0A, // push function literal onto stack
	O_CCONST = 0x0B, // push closure literal onto stack

	O_HALT = 0x0F, // halt

	O_MATCH = 0x31, // pattern matching

	O_BOR = 0x40, // bitwise or
	O_BXOR = 0x41, // bitwise xor
	O_BAND = 0x42, // bitwise and
	O_BANDNOT = 0x43, // bitwise and, with bitwise not on right operand
	O_BNOT = 0x44, // bit negation
	O_BSL = 0x45, // bitwise left shift
	O_BSR = 0x46, // bitwise right shift

	O_ASS = 0x50, // assert

	O_ADD = 0x60, // add two numbers
	O_SUB = 0x61, // subtract two numbers
	O_MUL = 0x62, // multiply two integers
	O_EXP = 0x63, // exponentiation
	O_FDIV = 0x64, // divide two numbers (return float)
	O_IDIV = 0x65, // divide two ints (return int)
	O_MOD = 0x66, // modulo two integers
	O_NEG = 0x67, // negate a number
	O_POS = 0x68, // positive of a number
	O_NOT = 0x69, // negate a boolean
	O_LEN = 0x6A, // get length
	O_CNCT = 0x6B, // concat two strings or lists

	O_LT = 0x70, // less than
	O_LE = 0x71, // less than or equal
	O_GT = 0x72, // greater than
	O_GE = 0x73, // greater than or equal
	O_EQ = 0x74, // equality
	O_ID = 0x76, // identity

	O_SET = 0x80, // sets field.
	O_GET = 0x88, // gets field.
	O_SLICE = 0x8A, // slice of list or str

	O_EXPORT = 0x90, // export

	O_LIT = 0x9A,
	O_LIT8 = 0x9B, // make new constant and push it onto stack (index into constant table: 8 bytes)
	O_NEWTABLE = 0x9C, // make new table and push it onto stack
	O_NEWLIST = 0x9D, // make new list and push it onto stack

	O_MOVEUP_FP = 0xA0, // move an element from index whatever to top of stack, indexing from fp.
	O_MOVEDOWN_FP = 0xA1,

	O_END = 0xB0, // indicate end of list on stack.
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

enum SpecialStrings {
#define X(E, S, ...) E,
#include "specialstrings.x"
#undef X

	NUM_SPECIAL_STRINGS // don't treat this as a member
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
