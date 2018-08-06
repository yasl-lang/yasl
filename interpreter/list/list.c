#include <inttypes.h>
#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include "list.h"
#include "../YASL_Object/YASL_Object.h"
#include <bytebuffer.h>
#include <bytebuffer/bytebuffer.h>

#define LS_BASESIZE 4

int isvalueinarray(int64_t val, int64_t *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return 1;
    }
    return 0;
}

YASL_Object YASL_List(List_t *ls) {
    return (YASL_Object) { .type = Y_LIST, .value.lval = ls };
}

List_t* ls_new_sized(const int base_size) {
    List_t* ls = malloc(sizeof(List_t));
    ls->size = base_size;
    ls->count = 0;
    ls->items = malloc(sizeof(YASL_Object)*ls->size);
    ls->rc = rc_new();
    return ls;
}

List_t* ls_new(void) {
    return ls_new_sized(LS_BASESIZE);
}

void ls_del_data(List_t *ls) {
    for (int i = 0; i < ls->count; i++) dec_ref(ls->items + i);
    free(ls->items);
}

void ls_del_rc(List_t *ls) {
    rc_del(ls->rc);
    free(ls);
}

void ls_del(List_t *ls) {
    for (int i = 0; i < ls->count; i++) dec_ref(ls->items + i);
    free(ls->items);
    rc_del(ls->rc);
    free(ls);
}

static void ls_resize(List_t* ls, const int base_size) {
    if (base_size < LS_BASESIZE) return;
    List_t* new_ls = ls_new_sized(base_size);
    int i;
    for (i = 0; i < ls->size; i++) {
        new_ls->items[i] = ls->items[i];
    }
    ls->size = new_ls->size;

    YASL_Object* tmp_items = ls->items;
    ls->items = new_ls->items;
    new_ls->items = tmp_items;

    ls_del(new_ls);
}

static void ls_resize_up(List_t* ls) {
    const int new_size = ls->size * 2;
    ls_resize(ls, new_size);
}

static void ls_resize_down(List_t* ls) {
    const int new_size = ls->size / 2;
    ls_resize(ls, new_size);
}

void ls_insert(List_t* ls, const int64_t index, const YASL_Object value) {
    if (ls->count >= ls->size) ls_resize_up(ls);
    ls->items[index] = value;
    ls->count++;
}

void ls_append(List_t* ls, const YASL_Object value) {
    if (ls->count >= ls->size) ls_resize_up(ls);
    ls->items[ls->count++] = value;
}

YASL_Object ls_search(List_t* ls, int64_t index) {
    if (index < -ls->count || index >= ls->count) return UNDEF_C;
    else if (0 <= index) return ls->items[index];
    else return ls->items[ls->count+index];
}

void ls_reverse(List_t *ls) {
    int64_t i;
    for(i = 0; i < ls->count/2; i++) {
        YASL_Object tmp = ls->items[i];
        ls->items[i] = ls->items[ls->count-i-1];
        ls->items[ls->count-i-1] = tmp;
    }
}

void ls_print(List_t* ls) {
    ByteBuffer *seen = bb_new(sizeof(int64_t)*2);
    ls_print_h(ls, seen);
}

void ls_print_h(List_t* ls, ByteBuffer *seen) {
    int i = 0;
    if (ls->count == 0) {
        printf("[]");
        return;
    }
    printf("[");
    while (i < ls->count) {
        if (yasl_type_equals(ls->items[i].type, Y_LIST)) {
            if (isvalueinarray(ls->items[i].value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("[...]");
            } else {
                bb_intbytes8(seen, (int64_t)ls);
                bb_intbytes8(seen, ls->items[i].value.ival);
                ls_print_h(ls->items[i].value.lval, seen);
            }
        } else if (yasl_type_equals(ls->items[i].type, Y_TABLE)) {
            if (isvalueinarray(ls->items[i].value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("[...->...]");
            } else {
                bb_intbytes8(seen, (int64_t)ls);
                bb_intbytes8(seen, ls->items[i].value.ival);
                ht_print_h(ls->items[i].value.mval, seen);
            }
        } else {
            print(ls->items[i]);
        }
        printf(", ");
        i++;
    }
    printf("\b\b]");
}
