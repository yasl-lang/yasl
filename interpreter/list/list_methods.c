#include "list_methods.h"

int list_append(VM* vm) {
    Constant ls  = POP(vm);
    Constant val = POP(vm);
    ls_append((List_t*)ls.value, val);
    PUSH(vm, UNDEF_C);
    return 0;
}