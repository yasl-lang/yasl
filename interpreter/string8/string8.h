#pragma once

#include <inttypes.h>

typedef struct {
    int64_t length;
    char* str;
} String_t;

/*
void ls_insert(List_t* ls, int64_t index, Constant value);
void ls_append(List_t* ls, Constant value);
Constant ls_search(List_t* ls, int64_t index);
void ls_delete(List_t* ls, int64_t index);
*/