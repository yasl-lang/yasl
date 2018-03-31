#pragma once

//#include "builtins/builtins.h"
#include "vtable/vtable.h"
#include "constant/constant.h"
#define STACK_SIZE 256
#define NUM_TYPES 6                                      // number of builtin types, each needs a vtable
#define PUSH(vm, v)  (vm->stack[++vm->sp] = v)           // push value onto stack
#define POP(vm)      (vm->stack[vm->sp--])               // pop value from top of stack
#define PEEK(vm)     (vm->stack[vm->sp])                 // pop value from top of stack
#define BPUSH(vm, v) (PUSH(vm, ((Constant) {BOOL, v})))  //push boolean v onto stack

typedef struct {
	Constant* globals;
	char* code;                 // bytecode
	Constant* stack;            // stack, see "constant.c" for details on Constant.
	int pc;                     // program counter
	int sp;                     // stack pointer
	int fp;                     // frame pointer
	VTable_t** builtins_vtable;   // vtable of builtin methods
} VM;

VM* newVM(char* code,    // pointer to bytecode
    int pc,              // address of instruction to be executed first -- entrypoint
    int datasize);       // total locals size required to perform a program operations

void delVM(VM* vm);
VTable_t* float64_builtins(void);
VTable_t* int64_builtins(void);
VTable_t* str8_builtins(void);
VTable_t* list_builtins(void);
VTable_t* hash_builtins(void);
VTable_t* file_builtins(void);