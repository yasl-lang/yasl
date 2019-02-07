#include "float_methods.h"

#include "yasl_float.h"
#include "yasl_state.h"
#include "VM.h"

int float_toint(struct YASL_State *S) {
    ASSERT_TYPE((struct VM *)S, Y_FLOAT, "float.toint64");
    struct YASL_Object a = vm_pop((struct VM *)S);
    vm_push((struct VM *)S, YASL_INT((yasl_int)YASL_GETFLOAT(a)));
    return 0;
}

int float_tostr(struct YASL_State *S) {
    ASSERT_TYPE((struct VM *)S, Y_FLOAT, "float.tostr");
    yasl_float val = YASL_GETFLOAT(vm_pop((struct VM *)S));
    char *ptr = float64_to_str(val);
    String_t* string = str_new_sized_heap(0, strlen(ptr), ptr);
    vm_push((struct VM *)S, YASL_STR(string));
    return 0;
}