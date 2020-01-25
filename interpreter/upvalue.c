#include "upvalue.h"

void vm_insert_upval(struct VM *const vm, const size_t offset, struct Upvalue *const upval) {
    if (vm->pending == NULL) {
        vm->pending = upval;
        return;
    }
    struct Upvalue *prev = NULL;
    struct Upvalue *curr = vm->pending;
    struct YASL_Object *index = vm->global_vars + offset;
    while (curr && curr->location > upval) {
        prev = curr;
        curr = curr->next;
    }
    prev->next = upval;
    upval->next = curr;
}
