#include <stdio.h>
#include <stdlib.h>
#include "VM.h"

VM* newVM(char* code,    // pointer to bytecode
    int pc0,             // address of instruction to be executed first -- entrypoint
    int datasize) {      // total locals size required to perform a program operations
    VM* vm = malloc(sizeof(VM));
    vm->code = code;
    vm->pc = pc0;
    vm->pc0 = vm->pc;
    vm->fp = 0;
    vm->sp = -1;
    vm->globals = malloc(sizeof(YASL_Object) * datasize);
    vm->globals[0] = (YASL_Object) {FILEH, (int64_t)stdin};
    vm->globals[1] = (YASL_Object) {FILEH, (int64_t)stdout};
    vm->globals[2] = (YASL_Object) {FILEH, (int64_t)stderr};
    vm->stack = malloc(sizeof(YASL_Object) * STACK_SIZE);
    vm->builtins_vtable = malloc(sizeof(VTable_t*) * NUM_TYPES);
    vm->builtins_vtable[1] = float64_builtins();
    vm->builtins_vtable[2] = int64_builtins();
    vm->builtins_vtable[3] = bool_builtins();
    vm->builtins_vtable[4] = str8_builtins();
    vm->builtins_vtable[5] = list_builtins();
    vm->builtins_vtable[6] = map_builtins();
    vm->builtins_vtable[7] = file_builtins();
    return vm;
}

void delVM(VM* vm){
    free(vm->globals);                   // TODO: free these properly
    free(vm->stack);                     // TODO: free these properly
    //del_vtable(vm->builtins_vtable[0]);
    del_vtable(vm->builtins_vtable[1]);
    del_vtable(vm->builtins_vtable[2]);
    del_vtable(vm->builtins_vtable[3]);
    del_vtable(vm->builtins_vtable[4]);
    del_vtable(vm->builtins_vtable[5]);
    del_vtable(vm->builtins_vtable[6]);
    del_vtable(vm->builtins_vtable[7]);
    free(vm->builtins_vtable);
        free(vm);
}