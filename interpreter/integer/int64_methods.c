#include "int64_methods.h"

int int64_tofloat64(VM* vm) {
    ASSERT_TYPE(vm, Y_INT64, "int64.tofloat64");
    int64_t val = POP(vm).value.ival;
    double d = (double)val;
    vm->stack[++vm->sp].type = Y_FLOAT64;
    memcpy(&vm->stack[vm->sp].value, &d, sizeof(d));
    return 0;
};

int int64_tostr(VM *vm) {
    ASSERT_TYPE(vm, Y_INT64, "int64.tostr");
    int64_t val = POP(vm).value.ival;
    char *ptr = malloc(snprintf(NULL, 0, "%" PRId64 "", val));
    sprintf(ptr, "%" PRId64 "", val);
    String_t* string = str_new_sized(snprintf(NULL, 0, "%" PRId64 "", val), ptr);
    PUSH(vm, ((YASL_Object){Y_STR, (int64_t)string}));
    return 0;
}