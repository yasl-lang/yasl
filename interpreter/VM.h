#pragma once

#include "IO.h"
#include "data-structures/YASL_Table.h"
#include "data-structures/YASL_List.h"
#include "yasl_conf.h"
#include "opcode.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define NUM_GLOBALS 256
#define NUM_TYPES 13                                     // number of builtin types, each needs a vtable

#define VM_PUSH(vm, x) do {struct YASL_Object to = x; vm_push(vm, to);} while(0)

#define VM_PEEK(vm, offset) ((vm)->stack[offset])
#define vm_peek(vm) ((vm)->stack[(vm)->sp])

#define vm_peekint(vm, offset) (YASL_GETINT(VM_PEEK(vm, offset)))
#define vm_peekstr(vm, offset) (YASL_GETSTR(VM_PEEK(vm, offset)))
#define vm_peeklist(vm, offset) (YASL_GETLIST(VM_PEEK(vm, offset)))
#define vm_peektable(vm, offset) (YASL_GETTABLE(VM_PEEK(vm, offset)))
#define vm_peekcfn(vm, offset) (YASL_GETCFN(VM_PEEK(vm, offset)))

#define vm_isend(vm) (YASL_ISEND(vm_peek(vm)))
#define vm_isundef(vm) (YASL_ISUNDEF(vm_peek(vm)))
#define vm_isfloat(vm) (YASL_ISFLOAT(vm_peek(vm)))
#define vm_isint(vm) (YASL_ISINT(vm_peek(vm)))
#define vm_isbool(vm) (YASL_ISBOOL(vm_peek(vm)))
#define vm_isstr(vm) (YASL_ISSTR(vm_peek(vm)))

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
                            else if (a.type == Y_FLOAT && b.type == Y_FLOAT) {\
                                c = f(a.value.dval, (b).value.dval);\
                            }\
                            else {\
                                printf("TypeError: %s not supported for operands of types %s and %s.\n", str,\
                                        YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);\
                                return YASL_TYPE_ERROR;\
                            }\
                            vm_pushbool(vm, c);} while(0);

#define vm_print_out(vm, format, ...) {\
	char *tmp = (char *)malloc(snprintf(NULL, 0, format, __VA_ARGS__) + 1);\
	sprintf(tmp, format, __VA_ARGS__);\
	vm->out.print(&vm->out, tmp, strlen(tmp));\
	free(tmp);\
}

#define vm_print_err(vm, format, ...) {\
	char *tmp = (char *)malloc(snprintf(NULL, 0, format, __VA_ARGS__) + 1);\
	sprintf(tmp, format, __VA_ARGS__);\
	(vm)->err.print(&(vm)->err, tmp, strlen(tmp));\
	free(tmp);\
}

#define vm_print_err_type(vm, format, ...) vm_print_err(vm, MSG_TYPE_ERROR format, __VA_ARGS__)
#define vm_print_err_divide_by_zero(vm) {\
	const char *tmp = "DivisionByZeroError\n";\
	vm->err.print(&vm->err, tmp, strlen(tmp));\
}
#define vm_print_err_bad_arg_type(vm, name, position, expected, actual) \
vm_print_err_type((vm),\
 "%s expected arg in position %d to be of type %s, got arg of type %s.\n",\
 name,\
 position,\
 YASL_TYPE_NAMES[expected],\
 YASL_TYPE_NAMES[actual])

struct VM {
	struct IO out;
	struct IO err;
	struct YASL_Object *global_vars;
	struct YASL_Table **globals;         // variables, see "constant.c" for details on YASL_Object.
	size_t num_globals;
	struct YASL_Object *stack;            // stack
	unsigned char *code;           // bytecode
	unsigned char **headers;
	size_t headers_size;
	unsigned char *pc;                     // program counter
	int sp;                        // stack pointer
	int fp;                        // frame pointer
	int next_fp;
	int lp;                        // foreach pointer
	struct YASL_String *special_strings[NUM_SPECIAL_STRINGS];
	struct YASL_Table **builtins_htable;   // htable of builtin methods
};

void vm_init(struct VM *const vm, unsigned char *const code, const size_t pc, const size_t datasize);

void vm_cleanup(struct VM *vm);

int vm_stringify_top(struct VM *vm);

struct YASL_Object vm_pop(struct VM *const vm);
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
#define vm_pushstr(vm, s) VM_PUSH(vm, YASL_STR(s))
#define vm_pushlist(vm, l) VM_PUSH(vm, YASL_LIST(l))
#define vm_pushtable(vm, l) VM_PUSH(vm, YASL_TABLE(l))
#define vm_pushfn(vm, f) VM_PUSH(vm, YASL_FN(f))

int vm_run(struct VM *vm);

struct YASL_Table *undef_builtins(struct VM *vm);
struct YASL_Table *float_builtins(struct VM *vm);
struct YASL_Table *int_builtins(struct VM *vm);
struct YASL_Table *bool_builtins(struct VM *vm);
struct YASL_Table *str_builtins(struct VM *vm);
struct YASL_Table *list_builtins(struct VM *vm);
struct YASL_Table *table_builtins(struct VM *vm);
