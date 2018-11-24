#include "bool_methods.h"
#include "yasl_state.h"

int bool_tostr(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_BOOL, "bool.tostr");
    int64_t val = vm_pop(S->vm).value.ival;
    String_t* string;
    if (val == 0) {
        string = str_new_sized(strlen("false"), copy_char_buffer(strlen("false"), "false"));
    } else {
        string = str_new_sized(strlen("true"), copy_char_buffer(strlen("true"), "true"));
    }
    vm_push(S->vm, YASL_STR(string)); // YASL_String(string));
    return 0;
}