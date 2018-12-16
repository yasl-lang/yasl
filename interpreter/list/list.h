#pragma once

#include "refcount.h"
#include "bytebuffer.h"

struct List {
    int64_t size;
    int64_t count;
    struct YASL_Object *items;
};

struct RC_List {
    struct RC *rc;      // RC MUST BE THE FIRST MEMBER OF LIST. DO NOT REARRANGE.
    struct List list;
};

struct YASL_Object *YASL_List(struct RC_List *ls);
int isvalueinarray(int64_t val, int64_t *arr, int size);
struct RC_List *ls_new(void);
struct RC_List* ls_new_sized(const int base_size);
void ls_del_data(struct RC_List *ls);
void ls_del_rc(struct RC_List *ls);
void ls_insert(struct RC_List* ls, int64_t index, struct YASL_Object value);
void ls_append(struct RC_List* ls, struct YASL_Object value);
struct YASL_Object ls_search(struct RC_List *ls, int64_t index);
void ls_reverse(struct RC_List *ls);
void ls_print(struct RC_List* ls);
void ls_print_h(struct RC_List* ls, ByteBuffer *seen);