#pragma once

#include "refcount.h"
#include "bytebuffer/bytebuffer.h"
#include "userdata.h"

#define LS_BASESIZE 4
#define FOR_LIST(i, name, list) struct YASL_Object name; for (size_t i = 0; i < (list)->count && (name = (list)->items[i], 1); i++)

struct List {
    size_t size;
    size_t count;
    struct YASL_Object *items;
};

int isvalueinarray(int64_t val, int64_t *arr, int size);
struct RC_UserData *ls_new(void);
struct RC_UserData* ls_new_sized(const size_t base_size);
void ls_del_data(void *ls);
void ls_insert(struct List* ls, int64_t index, struct YASL_Object value);
void ls_append(struct List* ls, struct YASL_Object value);
struct YASL_Object ls_search(struct List *ls, int64_t index);
void ls_reverse(struct List *ls);
