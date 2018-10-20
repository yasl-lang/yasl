#include "int64_methods.h"

int int64_tofloat64(struct VM* vm) {
    ASSERT_TYPE(vm, Y_INT64, "int64.tofloat64");
    int64_t val = vm_pop(vm).value.ival;
    vm_push(vm, YASL_Float((double)val));
    return 0;
};

int int64_tostr(struct VM *vm) {
    ASSERT_TYPE(vm, Y_INT64, "int64.tostr");
    int64_t val = vm_pop(vm).value.ival;
    unsigned char *ptr = malloc(snprintf(NULL, 0, "%" PRId64 "", val));
    sprintf(ptr, "%" PRId64 "", val);
    String_t* string = str_new_sized(snprintf(NULL, 0, "%" PRId64 "", val), ptr);
    vm_push(vm, YASL_String(string));
    return 0;
}