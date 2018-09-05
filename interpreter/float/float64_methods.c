#include <interpreter/YASL_Object/YASL_Object.h>
#include <ctype.h>
#include "float64_methods.h"

int float64_toint64(VM* vm) {
    ASSERT_TYPE(vm, Y_FLOAT64, "float.toint64");
    YASL_Object a = vm_pop(vm);
    vm_push(vm, YASL_Integer((int64_t)a.value.dval));
    return 0;
}

int float64_tostr(VM *vm) {
    ASSERT_TYPE(vm, Y_FLOAT64, "float.tostr");
    double val = vm_pop(vm).value.dval;
    int64_t size = snprintf(NULL, 0, "%f", val);
    char *ptr = malloc(size);
    sprintf(ptr, "%f", val);
    while (ptr[size-1] == '0' && ptr[size-2] != '.') {
        size--;
    }
    String_t* string = str_new_sized(size, ptr);
    vm_push(vm, YASL_String(string));
    return 0;
}