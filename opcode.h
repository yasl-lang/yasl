#pragma once

enum Opcode {
	HALT            = 0x0F, // halt
	NCONST          = 0x01, // push literal undef onto stack
	BCONST_F        = 0x08, // push literal false onto stack
	BCONST_T        = 0x09, // push literal true onto stack
	FCONST          = 0x0A, // push function literal onto stack

	ICONST          = 0x10, // push next 8 bytes onto stack as integer constant
	ICONST_M1       = 0x11, // push -1 onto stack
	ICONST_0        = 0x12, // push 0 onto stack
	ICONST_1        = 0x13, // push 1 onto stack
	ICONST_2        = 0x14, // push 2 onto stack
	ICONST_3        = 0x15, // push 3 onto stack
	ICONST_4        = 0x16, // push 4 onto stack
	ICONST_5        = 0x17, // push 5 onto stack
	DCONST          = 0x1A, // push next 8 bytes onto stack as float constant
	DCONST_0        = 0x1B, // push 0.0 onto stack
	DCONST_1        = 0x1C, // push 1.0 onto stack
	DCONST_2        = 0x1D, // push 2.0 onto stack
	DCONST_N        = 0x1E, // push nan onto stack
	DCONST_I        = 0x1F, // push inf onto stack

	BOR             = 0x40, // bitwise or
	BXOR            = 0x41, // bitwise xor
	BAND            = 0x42, // bitwise and
	BANDNOT         = 0x43, // bitwise and, with bitwise not on right operand
	BNOT            = 0x44, // bit negation
	BSL             = 0x45, // bitwise left shift
	BSR             = 0x46, // bitwise right shift

	ADD             = 0x60, // add two integers
	SUB             = 0x61, // subtract two integers
	MUL             = 0x62, // multiply two integers
	EXP             = 0x63, // exponentiation
	FDIV            = 0x64, // divide two integers
	IDIV            = 0x65, // divide two ints (return int)
	MOD             = 0x66, // modulo two numbers
	NEG             = 0x67, // negate an integer
	POS             = 0x68, // positive of a number
	NOT             = 0x69, // negate a boolean
	LEN             = 0x6A, // get length
	CNCT            = 0x6B, // concat two strings or lists

	GT              = 0x72, // greater than
	GE              = 0x73, // greater than or equal
	EQ              = 0x74, // equality
	ID              = 0x76, // identity

	SET             = 0x80, // sets field.
	GET             = 0x88, // gets field.
	SLICE           = 0x8A, // slice of list or str

	EXPORT          = 0x90, // export

	NEWSPECIALSTR   = 0x9A, // new special string.
	NEWSTR          = 0x9B, // make new String and push it onto stack (length (8 bytes), string (length bytes))
	NEWTABLE        = 0x9C, // make new HashTable and push it onto stack
	NEWLIST         = 0x9D, // make new List and push it onto stack

	END             = 0xB0, // indicate end of list on stack.
	DUP             = 0xB8, // duplicate top value of stack
	SWAP            = 0xBB, // swap top two elements of the stack
	POP             = 0xBF, // pop top of stack

	BR_8            = 0xC0, // branch unconditionally (takes next 8 bytes as jump length)
	BRF_8           = 0xC1, // branch if condition is falsey (takes next 8 bytes as jump length)
	BRT_8           = 0xC2, // branch if condition is truthy (takes next 8 bytes as jump length)
	BRN_8           = 0xC3, // branch if condition is not undef (takes next 8 bytes as jump length)

	INITFOR         = 0xD0, // initialises for-loop in VM
	ENDCOMP         = 0xD1, // end list / table comprehension
	ENDFOR          = 0xD2, // end for-loop in VM
	ITER_1          = 0xD3, // iterate to next, 1 var
	ITER_2          = 0xD5, // iterate to next, 2 var

	INIT_MC_SPECIAL = 0xE6,
	INIT_MC         = 0xE7,
	INIT_CALL       = 0xE8, // set up function call
	CALL            = 0xE9, // function call
	RET             = 0xEA, // return from function

	GSTORE_8        = 0xF0, // from string
	GLOAD_8         = 0xF1, // from string
	GSTORE_1        = 0xF4, // store top of stack at addr provided
	LSTORE_1        = 0xF5, // store top of stack as local at addr
	GLOAD_1         = 0xF6, // load global from addr
	LLOAD_1         = 0xF7, // load local from addr
	PRINT           = 0xFF  // print
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

	S_REP,     // repeat
	S_REPLACE,    // replace
	S_REVERSE,    // reverse
	S_RTRIM,      // rtrim

	S_SEARCH,     // search
	S_SLICE,      // slice
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
