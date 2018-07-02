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
#define NUM_TYPES 8                                      // number of builtin types, each needs a vtable
#define PUSH(vm, v)  (vm->stack[++vm->sp] = v)           // push value onto stack
#define POP(vm)      (vm->stack[vm->sp--])               // pop value from top of stack
#define PEEK(vm)     (vm->stack[vm->sp])                 // pop value from top of stack
#define BPUSH(vm, v) (PUSH(vm, ((YASL_Object) {Y_BOOL, v})))  //push boolean v onto stack


#define BUFFER_SIZE 256
#define NCODE(vm)    (vm->code[vm->pc++])     // get next bytecode
#define IPUSH(vm, v) (PUSH(vm, ((YASL_Object) {Y_INT64, v})))  //push integer v onto stack
#define IPOP(vm)     (((vm->stack)[vm->sp--])->value)      // get int from top of stack
#define IVAL(v)      (*((int64_t*)&v->value))
#define DPUSH(vm, v) (((FloatConstant*)vm->stack)[++vm->sp] = (FloatConstant) {Y_FLOAT64, v}) // push double v onto stack
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
	YASL_Object* globals;          // variables, see "constant.c" for details on YASL_Object.
	YASL_Object* stack;            // stack
	unsigned char* code;                 // bytecode
	int pc;                     // program counter
    int pc0;                    // initial value for pc
	int sp;                     // stack pointer
	int fp;                     // frame pointer
	Hash_t **builtins_htable;   // htable of builtin methods
} VM;

VM* vm_new(unsigned char *code,    // pointer to bytecode
           int pc0,             // address of instruction to be executed first -- entrypoint
           int datasize);       // total locals size required to perform a program operations

void vm_del(VM *vm);

void vm_run(VM *vm);

Hash_t* float64_builtins(void);
Hash_t* int64_builtins(void);
Hash_t* bool_builtins(void);
Hash_t* str_builtins(void);
Hash_t* list_builtins(void);
Hash_t* table_builtins(void);
Hash_t* file_builtins(void);
