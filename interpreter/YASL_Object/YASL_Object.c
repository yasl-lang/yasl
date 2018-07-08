#include <interpreter/YASL_string/YASL_string.h>
#include "YASL_Object.h"
#define DVAL(v)  (*((double*)&v.value))
#define TRUE_C   ((YASL_Object) {Y_BOOL, 1})
#define FALSE_C  ((YASL_Object) {Y_BOOL, 0})
#define UNDEF_C  ((YASL_Object) {Y_UNDEF, 0})


// Keep up to date with the YASL_Types
const char *YASL_TYPE_NAMES[] = {
    "undef",    //Y_UNDEF,
    "float64",  //Y_FLOAT64,
    "int64",    //Y_INT64,
    "bool",     //Y_BOOL,
    "str",      //Y_STR,
    "list",     //Y_LIST,
    "table",      //Y_TABLE,
    "file",     //Y_FILE,
    "fn",       //Y_FN
    "mn"        //Y_BFN
};


YASL_Object isequal(YASL_Object a, YASL_Object b) {
        if (a.type == Y_UNDEF || b.type == Y_UNDEF) {
            return UNDEF_C;
        }
        switch(a.type) {
        case Y_BOOL:
            if (b.type == Y_BOOL) {
                if (a.value.ival == b.value.ival) {
                    return TRUE_C;
                } else {
                    return FALSE_C;
                }
            } else {
                return FALSE_C;
            }
        case Y_TABLE:
            if (b.type == Y_TABLE) {
                puts("Warning: comparison of hashes currently is not implemented.");
                return UNDEF_C;
            }
            return FALSE_C;
        case Y_LIST:
            /*if (b.type == Y_LIST) {
                if (((List_t*)a.value)->count != ((List_t*)b.value)->count) {
                    return FALSE_C;
                } else {
                    int i = sizeof(int64_t);

                    while (i < *((int64_t*)a.value) + sizeof(int64_t)) {
                        if (a.type == b.type && a.value == b.value) {}
                        else if (FALSEY(isequal(((List_t*)a.value)->items[i], (List_t*)b.value)->items[i])) {
                            return FALSE_C;
                        }
                        i++;
                    }
                    return TRUE_C;
                }
            }
            return FALSE_C; */
            if (b.type == Y_LIST) {
                puts("Warning: comparison of lists currently is not implemented.");
                return UNDEF_C;
            }
            return FALSE_C;
        case Y_STR:
            if (b.type == Y_STR) {
                if (yasl_string_len(a.value.sval) != yasl_string_len(b.value.sval)) {
                    return FALSE_C;
                } else {
                    int i = 0;
                    while (i < yasl_string_len(a.value.sval)) {
                        if ((a.value.sval)->str.ptr[i] != (b.value.sval)->str.ptr[i]) {
                            return FALSE_C;
                        }
                        i++;
                    }
                    return TRUE_C;
                }
            }
            return FALSE_C;
        default:
            if (b.type == Y_BOOL || b.type == Y_TABLE) {
                return FALSE_C;
            }
            int c;
            if (a.type == Y_INT64 && b.type == Y_INT64) {
                c = a.value.ival == b.value.ival;
            } else if (a.type == Y_FLOAT64 && b.type == Y_INT64) {
                c = a.value.dval == (double)b.value.ival;
            } else if (a.type == Y_INT64 && b.type == Y_FLOAT64) {
                c = (double)a.value.ival == b.value.dval;
            } else if (a.type == Y_FLOAT64 && b.type == Y_FLOAT64) {
                c = a.value.dval == b.value.dval;
            } else {
                printf("== and != not supported for operands of types %x and %x.\n", a.type, b.type);
                return UNDEF_C;
            }
            return (YASL_Object) {Y_BOOL, c};
        }
}

int print(YASL_Object v) {
    int64_t i;
    switch (v.type) {
        case Y_INT64:
            printf("%" PRId64 "", v.value.ival);
            //printf("int64: %" PRId64 "\n", v.value);
            break;
        case Y_FLOAT64:
            printf("%f", *((double*)&v.value));
            //printf("float64: %f\n", *((double*)&v.value));
            break;
        case Y_BOOL:
            if (v.value.ival == 0) printf("false");
            else printf("true");
            break;
        case Y_UNDEF:
            printf("undef");
            break;
        case Y_STR:
            for (i = 0; i < yasl_string_len(v.value.sval); i++) {
                printf("%c", (v.value.sval)->str.ptr[i + v.value.sval->start]);
            }
            break;
        /* case Y_TABLE:
            printf("<hash %" PRIx64 ">", v.value);
            break; */
        /* case Y_LIST:
            //ls_print((List_t*)v.value);
            // printf("<list %" PRIx64 ">", v.value);
            break; */
        case Y_FILE:
            if (v.value.fval == stdin) {
                printf("stdin");
            } else if (v.value.fval == stdout) {
                printf("stdout");
            } else if (v.value.fval == stderr) {
                printf("stderr");
            } else {
                printf("<file %" PRIx64 ">", v.value.ival);
            }
            break;
        case Y_FN:
            printf("<fn: %" PRIx64 ">", v.value.ival);
            break;
        case Y_BFN:
            printf("<mn: %" PRIx64 ">", v.value.ival);
            break;
        default:
            printf("Error, unknown type: %x", v.type);
            return -1;
    }
    return 0;
}
