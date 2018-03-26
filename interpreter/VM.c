#include <stdio.h>
#include <stdlib.h>
#include "VM.h"

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
