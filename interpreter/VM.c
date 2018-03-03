#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "constant/constant.c"
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
    int datasize) {      // total locals size required to perform a program operations
    VM* vm = malloc(sizeof(VM));
    vm->code = code;
    vm->pc = pc;
    vm->fp = 0;
    vm->sp = -1;
    vm->globals = malloc(sizeof(Constant) * datasize);
    //vm->locals = malloc(sizeof(Constant) * datasize);
    vm->stack = malloc(sizeof(Constant) * STACK_SIZE);
    return vm;
}

void delVM(VM* vm){
        free(vm->globals);    // TODO: free these properly
        free(vm->stack);     // TODO: free these properly
        free(vm);
}
