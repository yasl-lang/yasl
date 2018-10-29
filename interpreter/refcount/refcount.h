#pragma once

#include <inttypes.h>

struct YASL_Object;

typedef struct {
    uint64_t refs;
    uint64_t weak_refs;
} RefCount;

RefCount *rc_new(void);
void rc_del(RefCount *rc);

void dec_ref(struct YASL_Object *v);
