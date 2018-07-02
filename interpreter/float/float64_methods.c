#include <interpreter/YASL_Object/YASL_Object.h>
#include "float64_methods.h"

int float64_toint64(VM* vm) {
    ASSERT_TYPE(vm, Y_FLOAT64, "float.toint64");
    YASL_Object a = POP(vm);
    vm->stack[++vm->sp].type = Y_INT64;
    PEEK(vm).value.ival = (int64_t)a.value.dval;
    return 0;
}

int float64_tostr(VM *vm) {
    ASSERT_TYPE(vm, Y_FLOAT64, "float.tostr");
    double val = 0;
    memcpy(&val, &vm->stack[vm->sp].value, sizeof(double));
    String_t* string = str_new_sized(snprintf(NULL, 0, "%f", val));
    sprintf(string->str, "%f", val);           // TODO: adjust so that it doesn't use a null terminator
    PEEK(vm) = (YASL_Object){Y_STR, (int64_t)string};
    return 0;
}