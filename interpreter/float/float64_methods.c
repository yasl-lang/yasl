#include "float64_methods.h"

int float64_toint64(VM* vm) {
    Constant a = POP(vm);
    double d;
    memcpy(&d, &a.value, sizeof(d));
    vm->stack[++vm->sp].type = INT64;
    PEEK(vm).value = (int64_t)d;
    return 0;
}

int float64_tostr(VM *vm) {
    double val = 0;
    memcpy(&val, &vm->stack[vm->sp].value, sizeof(double));
    String_t* string = new_sized_string8(snprintf(NULL, 0, "%f", val));
    sprintf(string->str, "%f", val);           // TODO: adjust so that it doesn't use a null terminator
    PEEK(vm) = (Constant){STR8, (int64_t)string};
    return 0;
}