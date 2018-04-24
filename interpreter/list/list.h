#pragma once

#include "../constant/constant.h"
#include "../hashtable/hashtable.h"
#define LEN(v) (*((int64_t*)v.value))

typedef struct {
    int size;
    int count;
    Constant* items;
} List_t;

int isvalueinarray(int64_t val, int64_t *arr, int size);
List_t* new_list(void);
void ls_insert(List_t* ls, int64_t index, Constant value);
void ls_append(List_t* ls, Constant value);
Constant ls_search(List_t* ls, int64_t index);
void ls_print(List_t* ls);
void ls_print_h(List_t* ls, int64_t* seen, int seen_size);