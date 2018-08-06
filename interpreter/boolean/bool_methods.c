#include "bool_methods.h"

int bool_tostr(VM *vm) {
    ASSERT_TYPE(vm, Y_BOOL, "bool.tostr");
    int64_t val = POP(vm).value.ival;
    String_t* string;
    if (val == 0) {
        string = str_new_sized(strlen("false"), "false");
        sprintf(string->str, "%s", "false");
    } else {
        string = str_new_sized(strlen("true"), "true");
        sprintf(string->str, "%s", "true");
    }
    vm_push(vm, ((YASL_Object){Y_STR, (int64_t)string}));
    return 0;
}