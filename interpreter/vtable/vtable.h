#pragma once

#include "../constant/constant.h"
#include <inttypes.h>

typedef struct {
    int64_t key;
    int64_t value;
} VItem_t;

struct VTable_s {
    struct VTable_s* parent;
    int64_t size;
    int64_t base_size;
    int64_t count;
    VItem_t** items;
};

typedef struct VTable_s VTable_t;

VTable_t* new_vtable(void);
void del_vtable(VTable_t* vtable);

void vt_insert(VTable_t* vtable, int64_t key, int64_t value);
int64_t vt_search(VTable_t* vtable, int64_t key);
void vt_delete(VTable_t* vtable, int64_t key);
