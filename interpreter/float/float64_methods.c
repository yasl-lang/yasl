#include <interpreter/YASL_Object/YASL_Object.h>
#include <ctype.h>
#include "float64_methods.h"
#include "float64.h"

int float64_toint64(struct VM* vm) {
    ASSERT_TYPE(vm, Y_FLOAT64, "float.toint64");
    struct YASL_Object a = vm_pop(vm);
    vm_push(vm, YASL_Integer((int64_t)a.value.dval));
    return 0;
}

int float64_tostr(struct VM *vm) {
    ASSERT_TYPE(vm, Y_FLOAT64, "float.tostr");
    double val = vm_pop(vm).value.dval;
    char *ptr = float64_to_str(val);
    String_t* string = str_new_sized(strlen(ptr), ptr);
    vm_push(vm, YASL_String(string));
    return 0;
}