#include "refcountptr.h"
#include <stdlib.h>

void rcptr_inc(rcptr ptr) {
    ++*ptr.count;
}

void rcptr_dec(rcptr ptr) {
    --*ptr.count;
    if (!*ptr.count) {
        //printf("deleted\n");
        free(ptr.ptr);
        free(ptr.count);
    }
}

rcptr rcptr_new(char *ptr) {
    int *count = malloc(sizeof(int));
    *count = 1;
    return (rcptr) { ptr, count };
}

rcptr rcptr_copy(rcptr ptr) {
    rcptr_inc(ptr);
    return ptr;
}
