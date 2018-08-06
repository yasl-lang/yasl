#include <stdlib.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include <interpreter/YASL_string/YASL_string.h>
#include <interpreter/list/list.h>
#include <color.h>
#include "refcount.h"



RefCount *rc_new(void) {
    RefCount *rc = malloc(sizeof(RefCount));
    rc->refs = 0;
    rc->weak_refs = 0;
    return rc;
}

void rc_del(RefCount *rc) {
    free(rc);
}

static void inc_weak_ref(YASL_Object *v) {
    printf(K_GRN "inc_weak(%s): ", YASL_TYPE_NAMES[v->type]);
    print(*v);
    puts(K_END);
    switch (v->type) {
        case Y_STR_W:
            v->value.sval->rc->weak_refs++;
            break;
        case Y_LIST_W:
            v->value.lval->rc->weak_refs++;
            break;
        case Y_TABLE_W:
            v->value.mval->rc->weak_refs++;
            break;
        default:
            puts("NOt Implemented");
            break;
    }
}

static void inc_strong_ref(YASL_Object *v) {
    printf(K_GRN "inc_strong(%s): ", YASL_TYPE_NAMES[v->type]);
    print(*v);
    puts(K_END);
    switch (v->type) {
        case Y_STR:
            v->value.sval->rc->refs++;
            break;
        case Y_LIST:
            v->value.lval->rc->refs++;
            break;
        case Y_TABLE:
            v->value.mval->rc->refs++;
            break;
        default:
            puts("NOt Implemented");
            break;
    }
}

void inc_ref(YASL_Object *v) {
    switch (v->type) {
        case Y_STR:
        case Y_LIST:
        case Y_TABLE:
            inc_strong_ref(v);
            break;
        case Y_STR_W:
        case Y_LIST_W:
        case Y_TABLE_W:
            inc_weak_ref(v);
            break;
        default:
            break;
    }
}

static void dec_weak_ref(YASL_Object *v) {
    printf(K_RED "dec_weak(%s): ", YASL_TYPE_NAMES[v->type]);
    print(*v);
    puts(K_END);
    switch(v->type) {
        case Y_STR_W:
            if (--(v->value.sval->rc->weak_refs) || v->value.sval->rc->refs) return;
            str_del_rc(v->value.sval);
            break;
        case Y_LIST_W:
            if (--(v->value.lval->rc->weak_refs) || v->value.lval->rc->refs) return;
            ls_del_rc(v->value.lval);
            break;
        case Y_TABLE_W:
            if (--(v->value.mval->rc->weak_refs) || v->value.mval->rc->refs) return;
            ht_del_rc(v->value.mval);
            break;
        default:
            puts("NoT IMPELemented");
            exit(EXIT_FAILURE);
    }
    *v = YASL_Undef();
}

void dec_strong_ref(YASL_Object *v) {
    printf(K_RED "dec_strong(%s): ", YASL_TYPE_NAMES[v->type]);
    print(*v);
    puts(K_END);
    switch(v->type) {
        case Y_STR:
            if (--(v->value.sval->rc->refs)) return;
            str_del_data(v->value.sval);
            if (v->value.sval->rc->weak_refs) return;
            str_del_rc(v->value.sval);
            break;
        case Y_LIST:
            if (--(v->value.lval->rc->refs)) return;
            ls_del_data(v->value.lval);
            if (v->value.lval->rc->weak_refs) return;
            ls_del_rc(v->value.lval);
            break;
        case Y_TABLE:
            if (--(v->value.mval->rc->refs)) return;
            ht_del_data(v->value.mval);
            if (v->value.mval->rc->weak_refs) return;
            ht_del_rc(v->value.mval);
            break;
        default:
            puts("NoT IMPELemented");
            exit(EXIT_FAILURE);
    }
}

void dec_ref(YASL_Object *v) {
    switch (v->type) {
        case Y_STR:
        case Y_LIST:
        case Y_TABLE:
            dec_strong_ref(v);
            break;
        case Y_STR_W:
        case Y_LIST_W:
        case Y_TABLE_W:
            dec_weak_ref(v);
            break;
        default:
            break;
    }
}