#include "list.h"

#include "YASL_Object.h"
#include "hashtable.h"

#define LS_BASESIZE 4

int isvalueinarray(int64_t val, int64_t *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return 1;
    }
    return 0;
}

struct YASL_Object *YASL_List(struct RC_List *ls) {
    struct YASL_Object *list = malloc(sizeof(struct YASL_Object));
    list->type = Y_LIST;
    list->value.lval = ls;
    return list;
}

struct RC_List* ls_new_sized(const int base_size) {
    struct RC_List* ls = malloc(sizeof(struct RC_List));
    ls->list = malloc(sizeof(struct List));
    ls->list->size = base_size;
    ls->list->count = 0;
    ls->list->items = malloc(sizeof(struct YASL_Object)*ls->list->size);
    ls->rc = rc_new();
    return ls;
}

struct RC_List* ls_new(void) {
    return ls_new_sized(LS_BASESIZE);
}

void ls_del_data(struct RC_List *ls) {
    for (int i = 0; i < ls->list->count; i++) dec_ref(ls->list->items + i);
    free(ls->list);
    free(ls->list->items);
}

void ls_del_rc(struct RC_List *ls) {
    rc_del(ls->rc);
    free(ls);
}

void ls_del(struct RC_List *ls) {
    for (int i = 0; i < ls->list->count; i++) dec_ref(ls->list->items + i);
    free(ls->list);
    free(ls->list->items);
    rc_del(ls->rc);
    free(ls);
}

static void ls_resize(struct List* ls, const int base_size) {
    if (base_size < LS_BASESIZE) return;
    struct RC_List* new_ls = ls_new_sized(base_size);
    int i;
    for (i = 0; i < ls->size; i++) {
        new_ls->list->items[i] = ls->items[i];
    }
    ls->size = new_ls->list->size;

    struct YASL_Object* tmp_items = ls->items;
    ls->items = new_ls->list->items;
    new_ls->list->items = tmp_items;

    ls_del(new_ls);
}

static void ls_resize_up(struct List* ls) {
    const int new_size = ls->size * 2;
    ls_resize(ls, new_size);
}

/*
static void ls_resize_down(struct RC_List* ls) {
    const int new_size = ls->list->size / 2;
    ls_resize(ls, new_size);
}
*/

void ls_insert(struct List* ls, const int64_t index, struct YASL_Object value) {
    if (ls->count >= ls->size) ls_resize_up(ls);
    dec_ref(ls->items+index);
    ls->items[index] = value;
    ls->count++;
    inc_ref(&value);
}

void ls_append(struct List* ls, struct YASL_Object value) {
    if (ls->count >= ls->size) ls_resize_up(ls);
    ls->items[ls->count++] = value;
    inc_ref(&value);
}

struct YASL_Object ls_search(struct List* ls, int64_t index) {
    if (index < -ls->count || index >= ls->count) return (struct YASL_Object) { .type = Y_UNDEF, .value.ival = 0 };
    else if (0 <= index) return ls->items[index];
    else return ls->items[ls->count+index];
}

void ls_reverse(struct List *ls) {
    int64_t i;
    for(i = 0; i < ls->count/2; i++) {
        struct YASL_Object tmp = ls->items[i];
        ls->items[i] = ls->items[ls->count-i-1];
        ls->items[ls->count-i-1] = tmp;
    }
}

/*
void ls_print(struct RC_List* ls) {
    ByteBuffer *seen = bb_new(sizeof(int64_t)*2);
    ls_print_h(ls, seen);
}

void ls_print_h(struct RC_List* ls, ByteBuffer *seen) {
    int i = 0;
    if (ls->list->count == 0) {
        printf("[]");
        return;
    }
    printf("[");
    while (i < ls->list->count) {
        if (YASL_ISLIST(ls->list->items[i])) {
            if (isvalueinarray(ls->list->items[i].value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("[...]");
            } else {
                bb_intbytes8(seen, (int64_t)ls);
                bb_intbytes8(seen, ls->list->items[i].value.ival);
                ls_print_h(ls->list->items[i].value.lval, seen);
            }
        } else if (YASL_ISTBL(ls->list->items[i])) {
            if (isvalueinarray(ls->list->items[i].value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("[...->...]");
            } else {
                bb_intbytes8(seen, (int64_t)ls);
                bb_intbytes8(seen, ls->list->items[i].value.ival);
                ht_print_h(ls->list->items[i].value.mval, seen);
            }
        } else {
            print(ls->list->items[i]);
        }
        printf(", ");
        i++;
    }
    printf("\b\b]");
}
*/
