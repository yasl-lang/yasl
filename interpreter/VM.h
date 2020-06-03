#ifndef YASL_VM_H_
#define YASL_VM_H_

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "IO.h"
#include "data-structures/YASL_Table.h"
#include "data-structures/YASL_List.h"
#include "yasl_conf.h"
#include "opcode.h"
// #include "closure.h"

#define NUM_GLOBALS 256
#define NUM_TYPES 13                                     // number of builtin types, each needs a vtable

// EXPAND is to deal with MSVC bullshit
#define GET_MACRO(_1, _2, NAME, ...) NAME
#define EXPAND(x) x
#define vm_peek_offset(vm, offset) ((vm)->stack[offset])
#define vm_peek_default(vm) ((vm)->stack[(vm)->sp])
#define vm_peek(...) EXPAND(GET_MACRO(__VA_ARGS__, vm_peek_offset, vm_peek_default,)(__VA_ARGS__))

#define vm_peekbool(...) EXPAND(YASL_GETBOOL(vm_peek(__VA_ARGS__)))
#define vm_peekfloat(...) EXPAND(YASL_GETFLOAT(vm_peek(__VA_ARGS__)))
#define vm_peekint(...) EXPAND(YASL_GETINT(vm_peek(__VA_ARGS__)))
#define vm_peekstr(...) EXPAND(YASL_GETSTR(vm_peek(__VA_ARGS__)))
#define vm_peeklist(...) (YASL_GETLIST(vm_peek(__VA_ARGS__)))
#define vm_peektable(...) (YASL_GETTABLE(vm_peek(__VA_ARGS__)))
#define vm_peekcfn(...) (YASL_GETCFN(vm_peek(__VA_ARGS__)))

#define vm_isend(vm) (YASL_ISEND(vm_peek(vm)))
#define vm_isundef(...) (YASL_ISUNDEF(vm_peek(__VA_ARGS__)))
#define vm_isfloat(vm) (YASL_ISFLOAT(vm_peek(vm)))
#define vm_isint(...) (YASL_ISINT(vm_peek(__VA_ARGS__)))
#define vm_isbool(vm) (YASL_ISBOOL(vm_peek(vm)))
#define vm_isstr(...) (YASL_ISSTR(vm_peek(__VA_ARGS__)))
#define vm_istable(vm) (YASL_ISTABLE(vm_peek(vm)))
#define vm_islist(...) (YASL_ISLIST(vm_peek(__VA_ARGS__)))
#define vm_isuserdata(vm) (YASL_ISUSERDATA(vm_peek(vm)))
#define vm_isuserptr(vm) (YASL_ISUSERPTR(vm_peek(vm)))

#define BUFFER_SIZE 256
#define NCODE(vm)    (*((vm)->pc++))     // get next bytecode

#define GT(a, b) ((a) > (b))
#define GE(a, b) ((a) >= (b))
#define COMP(vm, a, b, f, str)  do {\
                            if (a.type == Y_INT && b.type == Y_INT) {\
                                c = f(a.value.ival, b.value.ival);\
                            }\
                            else if (a.type == Y_FLOAT && b.type == Y_INT) {\
                                c = f(a.value.dval, (yasl_float)b.value.ival);\
                            }\
                            else if (a.type == Y_INT && b.type == Y_FLOAT) {\
                                c = f((yasl_float)a.value.ival, (b).value.dval);\
                            }\
                            else {\
                                c = f(a.value.dval, (b).value.dval);\
                            }\
                            vm_pushbool(vm, c);} while(0);

#define vm_print_err_type(vm, format, ...) vm_print_err((vm), MSG_TYPE_ERROR format, __VA_ARGS__)
#define vm_print_err_value(vm, format, ...) vm_print_err((vm), MSG_VALUE_ERROR format, __VA_ARGS__)
#define vm_print_err_divide_by_zero(vm) vm_print_err((vm), "DivisionByZeroError")
#define vm_print_err_bad_arg_type(vm, name, position, expected, actual) \
vm_print_err_type((vm),\
 "%s expected arg in position %d to be of type %s, got arg of type %s.",\
 name,\
 position,\
 YASL_TYPE_NAMES[expected],\
 YASL_TYPE_NAMES[actual])

struct CallFrame {
	unsigned char *pc;          // Where to reset the pc to after returning
	int prev_fp;                // Where to reset the fp to after returning
	int curr_fp;
	int lp;
};

struct LoopFrame {
	int iter;
	struct YASL_Object iterable;
};

struct VM {
	struct IO out;
	struct IO err;
	struct YASL_Table **globals;          // variables, see "constant.c" for details on YASL_Object.
	size_t num_globals;
	struct YASL_Object *stack;            // stack
	struct CallFrame frames[1000];
	int frame_num;
	struct LoopFrame loopframes[16];
	int loopframe_num;
	unsigned char *code;           // bytecode
	unsigned char **headers;
	size_t headers_size;
	unsigned char *pc;                     // program counter
	int sp;                        // stack pointer
	int fp;                        // frame pointer
	int next_fp;
	struct YASL_String *special_strings[NUM_SPECIAL_STRINGS];
	struct YASL_Table **builtins_htable;   // htable of builtin methods
	struct Upvalue *pending;
};

void vm_init(struct VM *const vm, unsigned char *const code, const size_t pc, const size_t datasize);

void vm_cleanup(struct VM *const vm);

void vvm_print_err(struct VM *vm, const char *const fmt, va_list args);
void vm_print_err(struct VM *vm, const char *const fmt, ...);

int vm_stringify_top(struct VM *const vm);

struct YASL_Object vm_pop(struct VM *const vm);
bool vm_popbool(struct VM *const vm);
yasl_float vm_popfloat(struct VM *const vm);
yasl_int vm_popint(struct VM *const vm);
struct YASL_String *vm_popstr(struct VM *const vm);
struct YASL_List *vm_poplist(struct VM *const vm);

void vm_push(struct VM *const vm, const struct YASL_Object val);
void vm_pushend(struct VM *const vm);
void vm_pushundef(struct VM *const vm);
void vm_pushfloat(struct VM *const vm, yasl_float f);
void vm_pushint(struct VM *const vm, yasl_int i);
void vm_pushbool(struct VM *const vm, bool b);
#define vm_pushstr(vm, s) vm_push(vm, YASL_STR(s))
#define vm_pushlist(vm, l) vm_push(vm, YASL_LIST(l))
#define vm_pushtable(vm, l) vm_push(vm, YASL_TABLE(l))
#define vm_pushfn(vm, f) vm_push(vm, YASL_FN(f))
void vm_pushclosure(struct VM *const vm, unsigned char *const f);

int vm_run(struct VM *const vm);

#endif
