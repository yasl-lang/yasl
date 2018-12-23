#include "float_methods.h"

#include "yasl_float.h"
#include "yasl_state.h"
#include "VM.h"

int float_toint(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_FLOAT64, "float.toint64");
    struct YASL_Object a = vm_pop(S->vm);
    vm_push(S->vm, YASL_INT((yasl_int)YASL_GETFLOAT(a)));
    return 0;
}

int float_tostr(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_FLOAT64, "float.tostr");
    yasl_float val = YASL_GETFLOAT(vm_pop(S->vm));
    char *ptr = float64_to_str(val);
    String_t* string = str_new_sized(strlen(ptr), ptr);
    vm_push(S->vm, YASL_STR(string));
    return 0;
}