#ifndef YASL_OPCODE_H_
#define YASL_OPCODE_H_

enum Opcode {
	O_HALT = 0x0F, // halt
	O_NCONST = 0x01, // push literal undef onto stack
	O_BCONST_F = 0x08, // push literal false onto stack
	O_BCONST_T = 0x09, // push literal true onto stack
	O_FCONST = 0x0A, // push function literal onto stack
	O_CCONST = 0x0B, // push closure literal onto stack

	O_ICONST = 0x10, // push next 8 bytes onto stack as integer constant
	O_ICONST_M1 = 0x11, // push -1 onto stack
	O_ICONST_0 = 0x12, // push 0 onto stack
	O_ICONST_1 = 0x13, // push 1 onto stack
	O_ICONST_2 = 0x14, // push 2 onto stack
	O_ICONST_3 = 0x15, // push 3 onto stack
	O_ICONST_4 = 0x16, // push 4 onto stack
	O_ICONST_5 = 0x17, // push 5 onto stack
	O_DCONST = 0x1A, // push next 8 bytes onto stack as float constant
	O_DCONST_0 = 0x1B, // push 0.0 onto stack
	O_DCONST_1 = 0x1C, // push 1.0 onto stack
	O_DCONST_2 = 0x1D, // push 2.0 onto stack

	O_ICONST_B1 = 0x20, // push literal integer that only requires 1 byte onto stack.
	O_ICONST_B2 = 0x21, // TODO
	O_ICONST_B4 = 0x22, // TODO
	O_ICONST_B8 = 0x23, // synonym for O_ICONST

	O_BOR = 0x40, // bitwise or
	O_BXOR = 0x41, // bitwise xor
	O_BAND = 0x42, // bitwise and
	O_BANDNOT = 0x43, // bitwise and, with bitwise not on right operand
	O_BNOT = 0x44, // bit negation
	O_BSL = 0x45, // bitwise left shift
	O_BSR = 0x46, // bitwise right shift

	O_ASS = 0x50, // assert

	O_ADD = 0x60, // add two integers
	O_SUB = 0x61, // subtract two integers
	O_MUL = 0x62, // multiply two integers
	O_EXP = 0x63, // exponentiation
	O_FDIV = 0x64, // divide two integers
	O_IDIV = 0x65, // divide two ints (return int)
	O_MOD = 0x66, // modulo two numbers
	O_NEG = 0x67, // negate an integer
	O_POS = 0x68, // positive of a number
	O_NOT = 0x69, // negate a boolean
	O_LEN = 0x6A, // get length
	O_CNCT = 0x6B, // concat two strings or lists

	O_GT = 0x72, // greater than
	O_GE = 0x73, // greater than or equal
	O_EQ = 0x74, // equality
	O_ID = 0x76, // identity

	O_SET = 0x80, // sets field.
	O_GET = 0x88, // gets field.
	O_SLICE = 0x8A, // slice of list or str

	O_EXPORT = 0x90, // export

	O_NEWSPECIALSTR = 0x9A, // new special string.
	O_NEWSTR = 0x9B, // make new String and push it onto stack (length (8 bytes), string (length bytes))
	O_NEWTABLE = 0x9C, // make new HashTable and push it onto stack
	O_NEWLIST = 0x9D, // make new List and push it onto stack

	O_CRET = 0xA0, // return from closure.

	O_END = 0xB0, // indicate end of list on stack.
	O_DUP = 0xB8, // duplicate top value of stack
	O_POP = 0xBF, // pop top of stack

	O_BR_8 = 0xC0, // branch unconditionally (takes next 8 bytes as jump length)
	O_BRF_8 = 0xC1, // branch if condition is falsey (takes next 8 bytes as jump length)
	O_BRT_8 = 0xC2, // branch if condition is truthy (takes next 8 bytes as jump length)
	O_BRN_8 = 0xC3, // branch if condition is not undef (takes next 8 bytes as jump length)

	O_INITFOR = 0xD0, // initialises for-loop in VM
	O_ENDCOMP = 0xD1, // end list / table comprehension
	O_ENDFOR = 0xD2, // end for-loop in VM
	O_ITER_1 = 0xD3, // iterate to next, 1 var
	O_ITER_2 = 0xD5, // iterate to next, 2 var

	O_INIT_MC_SPECIAL = 0xE6,
	O_INIT_MC = 0xE7,
	O_INIT_CALL = 0xE8, // set up function call
	O_CALL = 0xE9, // function call
	O_RET = 0xEA, // return from function

	O_GSTORE_8 = 0xF0, // from string
	O_GLOAD_8 = 0xF1, // from string
	O_USTORE_1 = 0xF2, // load upvalue
	O_ULOAD_1 = 0xF3, // store upvalue
	O_GSTORE_1 = 0xF4, // store top of stack at addr provided
	O_GLOAD_1 = 0xF5, // load global from addr
	O_LSTORE_1 = 0xF6, // store top of stack as local at addr
	O_LLOAD_1 = 0xF7, // load local from addr
	O_PRINT = 0xFF  // print
};

enum SpecialStrings {
	S_UNKNOWN_STR = -1, // ERROR, used internally but doesn't represent a real string value

	S___ADD,      // __add
	S___GET,      // __get
	S___SET,      // __set

	S_CLEAR,      // clear
	S_COPY,       // copy
	S_COUNT,      // count

	S_ENDSWITH,   // endswith
	S_EXTEND,     // extend

	S_ISAL,       // isal
	S_ISALNUM,    // isalnum
	S_ISNUM,      // isnum
	S_ISSPACE,    // isspace

	S_JOIN,       // join

	S_KEYS,       // keys

	S_LTRIM,      // ltrim

	S_POP,        // pop
	S_PUSH,       // push

	S_REMOVE,     // remove
	S_REP,        // repeat
	S_REPLACE,    // replace
	S_REVERSE,    // reverse
	S_RTRIM,      // rtrim

	S_SEARCH,     // search
	// S_SLICE,      // slice
	S_SORT,       // sort
	S_SPLIT,      // split
	S_STARTSWITH, // startswith

	S_TOBOOL,     // tobool
	S_TOFLOAT,    // tofloat
	S_TOINT,      // toint
	S_TOLOWER,    // tolower
	S_TOSTR,      // tostr
	S_TOUPPER,    // toupper
	S_TRIM,       // trim

	S_VALUES,     // values
	
	NUM_SPECIAL_STRINGS // don't treat this as a member
};

#endif
