#include <stdio.h>
#include <stdlib.h>
#include "constant.c"
#define STACK_SIZE 256

typedef struct {
	Constant* locals; 
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
    vm->locals = malloc(sizeof(Constant) * datasize);
    vm->stack = malloc(sizeof(Constant) * STACK_SIZE);
    return vm;
}

void delVM(VM* vm){
        free(vm->locals);
        free(vm->stack);
        free(vm);
}
