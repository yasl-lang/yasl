#pragma once

#include "../YASL_Object/YASL_Object.h"
#include "../../opcode.h"
#include "../../hashtable/hashtable.h"
#include "../YASL_string/YASL_string.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define STACK_SIZE 256
#define NUM_TYPES 13                                     // number of builtin types, each needs a vtable
#define PUSH(vm, v)  (vm->stack[++vm->sp] = v)           // push value onto stack
#define POP(vm)      (vm->stack[vm->sp--])               // pop value from top of stack
#define PEEK(vm)     (vm->stack[vm->sp])                 // pop value from top of stack
#define BPUSH(vm, v) (vm_push(vm, YASL_Boolean(v)))  //push boolean v onto stack


#define BUFFER_SIZE 256
#define NCODE(vm)    (vm->code[vm->pc++])     // get next bytecode
#define ADD(a, b)    (a + b)
#define DIV(a, b)    (a / b)
#define SUB(a, b)    (a - b)
#define MUL(a, b)    (a * b)
#define MOD(a, b)    (a % b)
#define EXP(a, b)    (pow(a, b))
#define GT(a, b)     (a > b)
#define GE(a, b)     (a >= b)
#define EQ(a, b)     (a == b)
#define BINOP(vm, a, b, f, str)  ({\
                            if (a.type == Y_INT64 && b.type == Y_INT64) {\
                                c = f(a.value.ival, b.value.ival);\
                                IPUSH(vm, c);\
                                break;\
                            }\
                            else if (a.type == Y_FLOAT64 && b.type == Y_INT64) {\
                                d = f(a.value.dval, (double)b.value.ival);\
                            }\
                            else if (a.type == Y_INT64 && b.type == Y_FLOAT64) {\
                                d = f((double)a.value.ival, b.value.dval);\
                            }\
                            else if (a.type == Y_FLOAT64 && b.type == Y_FLOAT64) {\
                                d = f(a.value.dval, b.value.dval);\
                            }\
                            else {\
                                printf("TypeError: %s not supported for operands of types %s and %s.\n", str,\
                                        YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);\
                                return;\
                            }\
                            DPUSH(vm, d);})
#define COMP(vm, a, b, f, str)  ({\
                            if (a.type == Y_INT64 && b.type == Y_INT64) {\
                                c = f(a.value.ival, b.value.ival);\
                            }\
                            else if (a.type == Y_FLOAT64 && b.type == Y_INT64) {\
                                c = f(a.value.dval, (double)b.value.ival);\
                            }\
                            else if (a.type == Y_INT64 && b.type == Y_FLOAT64) {\
                                c = f((double)a.value.ival, (b).value.dval);\
                            }\
                            else if (a.type == Y_FLOAT64 && b.type == Y_FLOAT64) {\
                                c = f(a.value.dval, (b).value.dval);\
                            }\
                            else {\
                                printf("TypeError: %s not supported for operands of types %s and %s.\n", str,\
                                        YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);\
                                return;\
                            }\
                            BPUSH(vm, c);})

typedef struct {
    YASL_Object *stack;
    int64_t *indices;
    int sp;
} LoopStack;

typedef struct {
	YASL_Object *globals;          // variables, see "constant.c" for details on YASL_Object.
    int64_t num_globals;
	YASL_Object *stack;            // stack
	unsigned char *code;           // bytecode
	int64_t pc;                    // program counter
    int64_t pc0;                   // initial value for pc
	int sp;                     // stack pointer
	int fp;                     // frame pointer
	Hash_t **builtins_htable;   // htable of builtin methods
    LoopStack *loopstack;
} VM;

VM* vm_new(unsigned char *code,    // pointer to bytecode
           int pc0,             // address of instruction to be executed first -- entrypoint
           int datasize);       // total params size required to perform a program operations

void vm_del(VM *vm);

void vm_push(VM *vm, YASL_Object val);

void vm_run(VM *vm);

Hash_t* float64_builtins(void);
Hash_t* int64_builtins(void);
Hash_t* bool_builtins(void);
Hash_t* str_builtins(void);
Hash_t* list_builtins(void);
Hash_t* table_builtins(void);
Hash_t* file_builtins(void);
