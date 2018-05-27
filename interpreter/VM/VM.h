#pragma once

#include "../vtable/vtable.h"
#include "../YASL_Object/YASL_Object.h"
#include "../../opcode.h"
#include "../../hashtable/hashtable.h"
#include "../YASL_string/YASL_string.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define STACK_SIZE 256
#define NUM_TYPES 8                                      // number of builtin types, each needs a vtable
#define PUSH(vm, v)  (vm->stack[++vm->sp] = v)           // push value onto stack
#define POP(vm)      (vm->stack[vm->sp--])               // pop value from top of stack
#define PEEK(vm)     (vm->stack[vm->sp])                 // pop value from top of stack
#define BPUSH(vm, v) (PUSH(vm, ((YASL_Object) {BOOL, v})))  //push boolean v onto stack


#define BUFFER_SIZE 256
#define NCODE(vm)    (vm->code[vm->pc++])     // get next bytecode
#define IPUSH(vm, v) (PUSH(vm, ((YASL_Object) {INT64, v})))  //push integer v onto stack
#define IPOP(vm)     (((vm->stack)[vm->sp--])->value)      // get int from top of stack
#define IVAL(v)      (*((int64_t*)&v->value))
#define DPUSH(vm, v) (((FloatConstant*)vm->stack)[++vm->sp] = (FloatConstant) {FLOAT64, v}) // push double v onto stack
#define LEN_C(v)     (*((int64_t*)v->value))
#define NPUSH(vm)    (PUSH(vm, ((YASL_Object) {UNDEF, 0})))   //push nil onto stack
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
                            if (a.type == INT64 && b.type == INT64) {\
                                c = f(a.value.ival, b.value.ival);\
                                IPUSH(vm, c);\
                                break;\
                            }\
                            else if (a.type == FLOAT64 && b.type == INT64) {\
                                d = f(a.value.dval, (double)b.value.ival);\
                            }\
                            else if (a.type == INT64 && b.type == FLOAT64) {\
                                d = f((double)a.value.ival, b.value.dval);\
                            }\
                            else if (a.type == FLOAT64 && b.type == FLOAT64) {\
                                d = f(a.value.dval, b.value.dval);\
                            }\
                            else {\
                                printf("TypeError: %s not supported for operands of types %s and %s.\n", str,\
                                        YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);\
                                return;\
                            }\
                            DPUSH(vm, d);})
#define COMP(vm, a, b, f, str)  ({\
                            if (a.type == INT64 && b.type == INT64) {\
                                c = f(a.value.ival, b.value.ival);\
                            }\
                            else if (a.type == FLOAT64 && b.type == INT64) {\
                                c = f(a.value.dval, (double)b.value.ival);\
                            }\
                            else if (a.type == INT64 && b.type == FLOAT64) {\
                                c = f((double)a.value.ival, (b).value.dval);\
                            }\
                            else if (a.type == FLOAT64 && b.type == FLOAT64) {\
                                c = f(a.value.dval, (b).value.dval);\
                            }\
                            else {\
                                printf("TypeError: %s not supported for operands of types %s and %s.\n", str,\
                                        YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);\
                                return;\
                            }\
                            BPUSH(vm, c);})

typedef struct {
	YASL_Object* globals;          // variables, see "constant.c" for details on YASL_Object.
	YASL_Object* stack;            // stack
	unsigned char* code;                 // bytecode
	int pc;                     // program counter
    int pc0;                    // initial value for pc
	int sp;                     // stack pointer
	int fp;                     // frame pointer
	VTable_t** builtins_vtable;   // vtable of builtin methods
} VM;

VM* newVM(unsigned char* code,    // pointer to bytecode
    int pc0,             // address of instruction to be executed first -- entrypoint
    int datasize);       // total locals size required to perform a program operations

void delVM(VM* vm);

void run(VM* vm);

VTable_t* float64_builtins(void);
VTable_t* int64_builtins(void);
VTable_t* bool_builtins(void);
VTable_t* str8_builtins(void);
VTable_t* list_builtins(void);
VTable_t* map_builtins(void);
VTable_t* file_builtins(void);

typedef int (*Handler)(VM*);
static const Handler builtins[];