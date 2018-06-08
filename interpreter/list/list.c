#include <inttypes.h>
#include  <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include "list.h"
#include "../YASL_Object/YASL_Object.h"
#define LS_BASESIZE 4

int isvalueinarray(int64_t val, int64_t *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return 1;
    }
    return 0;
}

List_t* new_sized_list(const int base_size) {
    List_t* ls = malloc(sizeof(List_t));
    ls->size = base_size;
    ls->count = 0;
    ls->items = malloc(sizeof(YASL_Object)*ls->size);
    return ls;
}

List_t* new_list() {
    return new_sized_list(LS_BASESIZE);
}

void del_list(List_t* ls) {
    free(ls->items);
    free(ls);
}

static void ls_resize(List_t* ls, const int base_size) {
    if (base_size < LS_BASESIZE) return;
    List_t* new_ls = new_sized_list(base_size);
    int i;
    for (i = 0; i < ls->size; i++) {
        //printf("ls->items[i].value: %d\n", (int)ls->items[i].value);
        new_ls->items[i] = ls->items[i];
    }
    ls->size = new_ls->size;
    //ls->count = new_ls->count;

    YASL_Object* tmp_items = ls->items;
    ls->items = new_ls->items;
    new_ls->items = tmp_items;

    del_list(new_ls);
}

static void ls_resize_up(List_t* ls) {
    //puts("resize up");
    const int new_size = ls->size * 2;
    ls_resize(ls, new_size);
    //printf("new size is: %d\n", ls->size);
}

static void ls_resize_down(List_t* ls) {
    //puts("resize down");
    const int new_size = ls->size / 2;
    ls_resize(ls, new_size);
    //printf("new size is: %d\n", ls->size);
}

void ls_insert(List_t* ls, const int64_t index, const YASL_Object value) {
    if (ls->count >= ls->size) ls_resize_up(ls);
    ls->items[index] = value;
    ls->count++;
}

void ls_append(List_t* ls, const YASL_Object value) {
    if (ls->count >= ls->size) ls_resize_up(ls);
    ls->items[ls->count++] = value;
    //ls->count++;
}

YASL_Object ls_search(List_t* ls, int64_t index) {
    if (index < 0 || index >= ls->size) return UNDEF_C; 
    return ls->items[index];
}

void ls_print(List_t* ls) {
    ls_print_h(ls, NULL,  0);
}

void ls_print_h(List_t* ls, int64_t* seen, int seen_size) {
    int i = 0;
    if (ls->count == 0) {
        printf("[]");
        return;
    }
    int64_t *new_seen;
    printf("[");
    while (i < ls->count) {
        if (ls->items[i].type == LIST) {
            if (isvalueinarray(ls->items[i].value.ival, seen, seen_size)) {
                printf("[...]");
                printf(", ");
            } else {
                new_seen = malloc(sizeof(int64_t) * (seen_size + 2));
                memcpy(new_seen, seen, seen_size);
                new_seen[seen_size] = (int64_t)ls;
                new_seen[seen_size + 1] = ls->items[i].value.ival;
                ls_print_h(ls->items[i].value.lval, new_seen, seen_size + 2);
                printf(", ");
                free(new_seen);
            }
        } else if (ls->items[i].type == MAP) {
            if (isvalueinarray(ls->items[i].value.ival, seen, seen_size)) {
                printf("[...->...]");
                printf(", ");
            } else {
                new_seen = malloc(sizeof(int64_t) * (seen_size + 2));
                memcpy(new_seen, seen, seen_size);
                new_seen[seen_size] = (int64_t)ls;
                new_seen[seen_size + 1] = ls->items[i].value.ival;
                ht_print_h(ls->items[i].value.mval, new_seen, seen_size + 2);
                printf(", ");
                free(new_seen);
            }
        } else {
            print(ls->items[i]);
            printf(", ");
        }
        i++;
    }
    /*if (ls->items[i].type == LIST) {
        if (isvalueinarray(ls->items[i].value, seen, seen_size)) {
            printf("[...]");
        } else {
            new_seen = malloc(sizeof(int64_t) * (seen_size + 2));
            memcpy(new_seen, seen, seen_size);
            new_seen[seen_size] = (int64_t)ls;
            new_seen[seen_size + 1] = ls->items[i].value;
            ls_print_h((List_t *) ls->items[i].value, new_seen, seen_size + 2);
            free(new_seen);
        }
    } else if (ls->items[i].type == MAP) {
        if (isvalueinarray(ls->items[i].value, seen, seen_size)) {
            printf("[...->...]");
        } else {
            new_seen = malloc(sizeof(int64_t) * (seen_size + 2));
            memcpy(new_seen, seen, seen_size);
            new_seen[seen_size] = (int64_t)ls;
            new_seen[seen_size + 1] = ls->items[i].value;
            ht_print_h((Hash_t *) ls->items[i].value, new_seen, seen_size + 2);
            free(new_seen);
        }
    } else {
        print(ls->items[i]);
    } */
    printf("\b\b]");
}