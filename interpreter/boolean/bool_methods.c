#include "bool_methods.h"

int bool_tostr(VM *vm) {
    int64_t val = POP(vm).value;
    String_t* string;
    if (val == 0) {
        string = new_sized_string8(snprintf(NULL, 0, "%s", "false"));
        sprintf(string->str, "%s", "false");
    } else {
        string = new_sized_string8(snprintf(NULL, 0, "%s", "true"));
        sprintf(string->str, "%s", "true");
    }
    PUSH(vm, ((Constant){STR8, (int64_t)string}));
    return 0;
}