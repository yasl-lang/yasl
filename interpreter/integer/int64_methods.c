#include "int64_methods.h"

int int64_tofloat64(VM* vm) {
    int64_t val = POP(vm).value;
    double d = (double)val;
    vm->stack[++vm->sp].type = FLOAT64;
    memcpy(&vm->stack[vm->sp].value, &d, sizeof(d));
    return 0;
};

int int64_tostr(VM *vm) {
    int64_t val = POP(vm).value;
    String_t* string = new_sized_string8(snprintf(NULL, 0, "%" PRId64 "", val));
    sprintf(string->str, "%" PRId64 "", val);           // TODO: adjust so that it doesn't use a null terminator
    PUSH(vm, ((YASL_Object){STR8, (int64_t)string}));
    return 0;
}