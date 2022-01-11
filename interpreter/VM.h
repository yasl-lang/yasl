#ifndef YASL_VM_H_
#define YASL_VM_H_

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>

#include "IO.h"
#include "data-structures/YASL_Table.h"
#include "data-structures/YASL_List.h"
#include "opcode.h"
#include "yapp.h"
#include "yasl_conf.h"


#define NUM_FRAMES 1000
#define NUM_TYPES 13                                     // number of builtin types, each needs a vtable
#define SCRATCH_SIZE 1024								// scratchspace size (needs tuning)

#define vm_peek_offset(vm, offset) ((vm)->stack[offset])
#define vm_peek_offset_p(vm, offset) ((vm)->stack + (offset))
#define vm_peek_default(vm) ((vm)->stack[(vm)->sp])
#define vm_peek_default_p(vm) ((vm)->stack + (vm)->sp)
#define vm_peek(...) YAPP_EXPAND(YAPP_CHOOSE2(__VA_ARGS__, vm_peek_offset, vm_peek_default,)(__VA_ARGS__))
#define vm_peek_p(...) YAPP_EXPAND(YAPP_CHOOSE2(__VA_ARGS__, vm_peek_offset_p, vm_peek_default_p,)(__VA_ARGS__))
#define vm_fppeek(vm, offset) ((vm)->stack[(vm)->fp + offset])

#define vm_peektypename(...) obj_typename(vm_peek_p(__VA_ARGS__))

#define vm_peekbool(...) YAPP_EXPAND(obj_getbool(vm_peek_p(__VA_ARGS__)))
#define vm_peekfloat(...) YAPP_EXPAND(obj_getfloat(vm_peek_p(__VA_ARGS__)))
#define vm_peekint(...) YAPP_EXPAND(obj_getint(vm_peek_p(__VA_ARGS__)))
#define vm_peekstr(...) YAPP_EXPAND(obj_getstr(vm_peek_p(__VA_ARGS__)))
#define vm_peeklist(...) (YASL_GETLIST(vm_peek(__VA_ARGS__)))
#define vm_peektable(...) (YASL_GETTABLE(vm_peek(__VA_ARGS__)))
#define vm_peekcfn(...) (YASL_GETCFN(vm_peek(__VA_ARGS__)))
#define vm_peekuserptr(...) YAPP_EXPAND(obj_getuserptr(vm_peek_p(__VA_ARGS__)))

#define vm_isend(vm) (YASL_ISEND(vm_peek(vm)))
#define vm_isundef(...) (obj_isundef(vm_peek_p(__VA_ARGS__)))
#define vm_isfloat(...) (obj_isfloat(vm_peek_p(__VA_ARGS__)))
#define vm_isint(...) (obj_isint(vm_peek_p(__VA_ARGS__)))
#define vm_isnum(...) (obj_isnum(vm_peek_p(__VA_ARGS__)))
#define vm_isbool(...) (obj_isbool(vm_peek_p(__VA_ARGS__)))
#define vm_isstr(...) (obj_isstr(vm_peek_p(__VA_ARGS__)))
#define vm_istable(...) (obj_istable(vm_peek_p(__VA_ARGS__)))
#define vm_islist(...) (obj_islist(vm_peek_p(__VA_ARGS__)))
#define vm_isuserdata(...) (obj_isuserdata(vm_peek_p(__VA_ARGS__)))
#define vm_isuserptr(...) (obj_isuserptr(vm_peek_p(__VA_ARGS__)))
#define vm_isfn(...) (obj_isfn(vm_peek_p(__VA_ARGS__)))
#define vm_iscfn(...) (obj_iscfn(vm_peek_p(__VA_ARGS__)))
#define vm_isclosure(...) (obj_isclosure(vm_peek_p(__VA_ARGS__)))

#define NCODE(vm)    (*((vm)->pc++))     // get next bytecode

#define GT(a, b) ((a) > (b))
#define GE(a, b) ((a) >= (b))
#define LT(a, b) ((a) < (b))
#define LE(a, b) ((a) <= (b))

#define COMP(vm, a, b, f)  do {\
	if (obj_isint(&a) && obj_isint(&b)) {\
		c = f(obj_getint(&a), obj_getint(&b));\
	} else {\
		c = f(obj_getnum(&a), obj_getnum(&b));\
	}\
	vm_pushbool(vm, c);} while(0);

#define vm_print_err_type(vm, format, ...) vm_print_err((vm), MSG_TYPE_ERROR format, __VA_ARGS__)
#define vm_print_err_value(vm, format, ...) vm_print_err((vm), MSG_VALUE_ERROR format, __VA_ARGS__)
#define vm_print_err_divide_by_zero(vm) vm_print_err((vm), "DivisionByZeroError")
#define vm_print_err_bad_arg_type_name(vm, name, position, expected, actual) \
vm_print_err_type((vm),\
 "%s expected arg in position %d to be of type %s, got arg of type %s.",\
 name,\
 position,\
 expected,\
 actual)

struct CallFrame {
	unsigned char *pc;          // Where to reset the pc to after returning
	int prev_fp;                // Where to reset the fp to after returning
	int curr_fp;
	int lp;
	int num_returns;
};

struct LoopFrame {
	int iter;
	struct YASL_Object iterable;
};

struct VM {
	struct IO out;
	struct IO err;
	struct YASL_Table *metatables;
	struct YASL_Table *globals;   // variables, see "constant.c" for details on YASL_Object.
	struct YASL_Object *stack;     // stack
	struct CallFrame frames[NUM_FRAMES];
	int frame_num;
	struct LoopFrame loopframes[16];
	int loopframe_num;
	struct YASL_Object *constants;
	int64_t num_constants;
	unsigned char *code;           // bytecode
	unsigned char **headers;
	size_t headers_size;
	unsigned char *pc;                     // program counter
	int sp;                        // stack pointer
	int fp;                        // frame pointer
	int next_fp;
	struct YASL_String *special_strings[NUM_SPECIAL_STRINGS];
	struct RC_UserData **builtins_htable;   // htable of builtin methods
	struct Upvalue *pending;
	jmp_buf buf;
	int status;
	uint8_t scratch[SCRATCH_SIZE];
};

void vm_init(struct VM *const vm, unsigned char *const code, const size_t pc, const size_t datasize);

void vm_cleanup(struct VM *const vm);

/*
 * These functions are used for declaring and freeing memory that may be used in a cycle, for example the memory for
 * list items (since a list could contain a reference to itself, creating a cycle).
 *
 * They support the same API as malloc/free.
 */
void *vm_alloc_cyclic(struct VM *vm, size_t size);
void vm_free_cyclic(struct VM *vm, void *ptr);

void vvm_print_err(struct VM *vm, const char *const fmt, va_list args);
void vm_print_err(struct VM *vm, const char *const fmt, ...);

YASL_NORETURN void vm_throw_err(struct VM *const vm, int error);

void vm_get_metatable(struct VM *const vm);
void vm_stringify_top(struct VM *const vm);
void vm_EQ(struct VM *const vm);

void vm_INIT_CALL_offset(struct VM *const vm, int offset, int expected_returns);
void vm_CALL(struct VM *const vm);

void vm_dec_ref(struct VM *const vm, struct YASL_Object *val);

struct YASL_Object vm_pop(struct VM *const vm);
bool vm_popbool(struct VM *const vm);
yasl_float vm_popfloat(struct VM *const vm);
yasl_int vm_popint(struct VM *const vm);
struct YASL_String *vm_popstr(struct VM *const vm);
struct YASL_List *vm_poplist(struct VM *const vm);
struct YASL_Table *vm_poptable(struct VM *const vm);

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

int vm_run(struct VM *const vm);

#endif
