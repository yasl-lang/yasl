#include "int64_methods.h"

int int64_tofloat64(VM* vm) {
    Constant a = POP(vm);
    double d = (double)a.value;
    vm->stack[++vm->sp].type = FLOAT64;
    memcpy(&vm->stack[vm->sp].value, &d, sizeof(d));
    return 0;
};