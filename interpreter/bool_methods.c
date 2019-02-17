#include "bool_methods.h"

#include "VM.h"
#include "YASL_Object.h"
#include "yasl_state.h"

int bool_tostr(struct YASL_State *S) {
    ASSERT_TYPE((struct VM *)S, Y_BOOL, "bool.tostr");
    int val = YASL_GETBOOL(vm_pop((struct VM *)S));
    String_t* string;
    if (val == 0) {
        string = str_new_sized(strlen("false"), "false");
    } else {
        string = str_new_sized(strlen("true"), "true");
    }
    vm_push((struct VM *)S, YASL_STR(string));
    return 0;
}
