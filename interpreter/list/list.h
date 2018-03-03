#pragma once

#include "../constant/constant.c"
#define LEN(v) (*((int64_t*)v.value))

typedef struct {
    int size;
    int count;
    Constant* items;
} List_t;

void ls_insert(List_t* ls, int64_t index, Constant value);
void ls_append(List_t* ls, Constant value);
Constant ls_search(List_t* ls, int64_t index);
void ls_delete(List_t* ls, int64_t index);
