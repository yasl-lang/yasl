#pragma once

#include "constant/constant.h"
#define STACK_SIZE 256
#define PUSH(vm, v)  (vm->stack[++vm->sp] = v)           // push value onto stack
#define POP(vm)      (vm->stack[vm->sp--])               // pop value from top of stack
#define PEEK(vm)     (vm->stack[vm->sp])                 // pop value from top of stack
#define BPUSH(vm, v) (PUSH(vm, ((Constant) {BOOL, v})))  //push boolean v onto stack

typedef struct {
	Constant* globals;
	char* code;         // bytecode
	Constant* stack;    // stack, see "constant.c" for details on Constant.
	int pc;             // program counter
	int sp;             // stack pointer
	int fp;             // frame pointer
} VM;

VM* newVM(char* code,    // pointer to bytecode
    int pc,              // address of instruction to be executed first -- entrypoint
    int datasize);       // total locals size required to perform a program operations

void delVM(VM* vm);
