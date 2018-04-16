#include "float64_methods.h"

int float64_toint64(VM* vm) {
    Constant a = POP(vm);
    double d;
    memcpy(&d, &a.value, sizeof(d));
    vm->stack[++vm->sp].type = INT64;
    PEEK(vm).value = (int64_t)d;
    return 0;
}
