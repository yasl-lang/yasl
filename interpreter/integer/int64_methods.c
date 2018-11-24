#include "int64_methods.h"
#include "yasl_state.h"

int int64_tofloat64(struct YASL_State* S) {
    ASSERT_TYPE(S->vm, Y_INT64, "int64.tofloat64");
    int64_t val = vm_pop(S->vm).value.ival;
    vm_push(S->vm, YASL_FLOAT((double)val));
    return 0;
};

int int64_tostr(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_INT64, "int64.tostr");
    int64_t val = vm_pop(S->vm).value.ival;
    unsigned char *ptr = malloc(snprintf(NULL, 0, "%" PRId64 "", val));
    sprintf(ptr, "%" PRId64 "", val);
    String_t* string = str_new_sized(snprintf(NULL, 0, "%" PRId64 "", val), ptr);
    vm_push(S->vm, YASL_STR(string));
    return 0;
}