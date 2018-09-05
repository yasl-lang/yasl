#include "bool_methods.h"

int bool_tostr(VM *vm) {
    ASSERT_TYPE(vm, Y_BOOL, "bool.tostr");
    int64_t val = vm_pop(vm).value.ival;
    String_t* string;
    if (val == 0) {
        string = str_new_sized(strlen("false"), copy_char_buffer(strlen("false"), "false"));
    } else {
        string = str_new_sized(strlen("true"), copy_char_buffer(strlen("true"), "true"));
    }
    vm_push(vm, YASL_String(string));
    return 0;
}