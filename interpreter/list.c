#include "list.h"

#include "YASL_Object.h"
#include "hashtable/hashtable.h"

int isvalueinarray(int64_t val, int64_t *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return 1;
    }
    return 0;
}

struct RC_UserData* ls_new_sized(const int base_size) {
    struct RC_UserData *ls = malloc(sizeof(struct RC_UserData));
    struct List *list = malloc(sizeof(struct List));
    list->size = base_size;
    list->count = 0;
    list->items = malloc(sizeof(struct YASL_Object)*list->size);
    ls->data = list;
    ls->rc = rc_new();
    ls->tag = T_LIST;
    return ls;
}

struct RC_UserData* ls_new(void) {
    return ls_new_sized(LS_BASESIZE);
}

void ls_del_data(struct RC_UserData *ls) {
    for (int i = 0; i < ((struct List*)ls->data)->count; i++) dec_ref(((struct List*)ls->data)->items + i);
    free(((struct List*)ls->data)->items);
    free(ls->data);
}

void ls_del_rc(struct RC_UserData *ls) {
    rc_del(ls->rc);
    free(ls);
}

void ls_del(struct RC_UserData *ls) {
    for (int i = 0; i < ((struct List *)ls->data)->count; i++) dec_ref(((struct List *)ls->data)->items + i);
    free(((struct List *)ls->data)->items);
    free(((struct List *)ls->data));
    rc_del(ls->rc);
    free(ls);
}

static void ls_resize(struct List* ls, const int base_size) {
    if (base_size < LS_BASESIZE) return;
    struct RC_UserData *new_ls = ls_new_sized(base_size);
    int i;
    for (i = 0; i < ls->size; i++) {
	    ((struct List *)new_ls->data)->items[i] = ls->items[i];
    }
    ls->size = ((struct List *)new_ls->data)->size;

    struct YASL_Object* tmp_items = ls->items;
    ls->items = ((struct List *)new_ls->data)->items;
	((struct List *)new_ls->data)->items = tmp_items;

    ls_del(new_ls);
}

static void ls_resize_up(struct List* ls) {
    const int new_size = ls->size * 2;
    ls_resize(ls, new_size);
}

/*
static void ls_resize_down(struct RC_List* ls) {
    const int new_size = ((struct List *)ls->data)->size / 2;
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
        } else if (YASL_ISTABLE(ls->list->items[i])) {
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
