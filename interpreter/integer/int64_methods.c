#include "int64_methods.h"

#include "VM.h"
#include "yasl_state.h"
#include "yasl_conf.h"

int int64_tofloat64(struct YASL_State* S) {
    ASSERT_TYPE(S->vm, Y_INT64, "int64.tofloat64");
    vm_push(S->vm, YASL_FLOAT((yasl_float)YASL_GETINT(vm_pop(S->vm))));
    return 0;
}

int int64_tostr(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_INT64, "int64.tostr");
    yasl_int val = YASL_GETINT(vm_pop(S->vm));
    char *ptr = malloc(snprintf(NULL, 0, "%" PRId64 "", val));
    sprintf(ptr, "%" PRId64 "", val);
    String_t* string = str_new_sized(snprintf(NULL, 0, "%" PRId64 "", val), ptr);
    vm_push(S->vm, YASL_STR(string));
    return 0;
}