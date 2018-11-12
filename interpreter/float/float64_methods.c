#include <interpreter/YASL_Object/YASL_Object.h>
#include <ctype.h>
#include "float64_methods.h"
#include "float64.h"
#include "yasl_state.h"

int float64_toint64(struct YASL_State* S) {
    ASSERT_TYPE(S->vm, Y_FLOAT64, "float.toint64");
    struct YASL_Object a = vm_pop(S->vm);
    vm_push(S->vm, YASL_Integer((int64_t)a.value.dval));
    return 0;
}

int float64_tostr(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_FLOAT64, "float.tostr");
    double val = vm_pop(S->vm).value.dval;
    char *ptr = float64_to_str(val);
    String_t* string = str_new_sized(strlen(ptr), ptr);
    vm_push(S->vm, YASL_String(string));
    return 0;
}