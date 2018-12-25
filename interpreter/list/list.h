#pragma once

#include "refcount.h"
#include "bytebuffer.h"
#include "userdata.h"

struct List {
    int64_t size;
    int64_t count;
    struct YASL_Object *items;
};

int isvalueinarray(int64_t val, int64_t *arr, int size);
struct RC_UserData *ls_new(void);
struct RC_UserData* ls_new_sized(const int base_size);
void ls_del_data(struct RC_UserData *ls);
void ls_del_rc(struct RC_UserData *ls);
void ls_insert(struct List* ls, int64_t index, struct YASL_Object value);
void ls_append(struct List* ls, struct YASL_Object value);
struct YASL_Object ls_search(struct List *ls, int64_t index);
void ls_reverse(struct List *ls);