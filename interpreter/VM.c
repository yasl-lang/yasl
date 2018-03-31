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
    vm->globals[0] = (Constant) {FILEH, (int64_t)stdin};
    vm->globals[1] = (Constant) {FILEH, (int64_t)stdout};
    vm->globals[2] = (Constant) {FILEH, (int64_t)stderr};
    vm->stack = malloc(sizeof(Constant) * STACK_SIZE);
    vm->builtins_vtable = malloc(sizeof(VTable_t*) * NUM_TYPES);
    vm->builtins_vtable[0] = float64_builtins();
    vm->builtins_vtable[1] = int64_builtins();
    vm->builtins_vtable[2] = str8_builtins();
    vm->builtins_vtable[3] = list_builtins();
    vm->builtins_vtable[4] = hash_builtins();
    vm->builtins_vtable[5] = file_builtins();
    return vm;
}

void delVM(VM* vm){
        free(vm->globals);                   // TODO: free these properly
        free(vm->stack);                     // TODO: free these properly
        del_vtable(vm->builtins_vtable[0]);
        del_vtable(vm->builtins_vtable[1]);
        del_vtable(vm->builtins_vtable[2]);
        del_vtable(vm->builtins_vtable[3]);
        del_vtable(vm->builtins_vtable[4]);
        del_vtable(vm->builtins_vtable[5]);
        free(vm->builtins_vtable);
        free(vm);
}
