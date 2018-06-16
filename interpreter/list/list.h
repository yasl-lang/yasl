#pragma once

#include <bytebuffer/bytebuffer.h>
#include "../YASL_Object/YASL_Object.h"
#include "../../hashtable/hashtable.h"
#define LEN(v) (*((int64_t*)v.value))

typedef struct List_s {
    int size;
    int count;
    YASL_Object* items;
} List_t;

int isvalueinarray(int64_t val, int64_t *arr, int size);
List_t *new_list(void);
List_t* new_sized_list(const int base_size);
void ls_insert(List_t* ls, int64_t index, YASL_Object value);
void ls_append(List_t* ls, YASL_Object value);
YASL_Object ls_search(List_t* ls, int64_t index);
void ls_print(List_t* ls);
void ls_print_h(List_t* ls, ByteBuffer *seen);