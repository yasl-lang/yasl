#include "bool_methods.h"

#include "VM.h"
#include "YASL_Object.h"
#include "yasl_state.h"

int bool_tostr(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_BOOL, "bool.tostr");
    int val = YASL_GETBOOL(vm_pop(S->vm));
    String_t* string;
    if (val == 0) {
        string = str_new_sized(strlen("false"), copy_char_buffer(strlen("false"), "false"));
    } else {
        string = str_new_sized(strlen("true"), copy_char_buffer(strlen("true"), "true"));
    }
    vm_push(S->vm, YASL_STR(string));
    return 0;
}
