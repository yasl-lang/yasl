#include "YASL_Object.h"
#define DVAL(v)  (*((double*)&v.value))
#define TRUE_C   ((YASL_Object) {BOOL, 1})
#define FALSE_C  ((YASL_Object) {BOOL, 0})
#define UNDEF_C  ((YASL_Object) {UNDEF, 0})


// Keep up to date with the YASL_Types
const char *YASL_TYPE_NAMES[] = {
    "undef",    //UNDEF,
    "float64",  //FLOAT64,
    "int64",    //INT64,
    "bool",     //BOOL,
    "str",      //STR8,
    "list",     //LIST,
    "map",      //MAP,
    "file",     //FILEH,
    "fn",       //FN_P
    "mn"        //MN_P
};


YASL_Object isequal(YASL_Object a, YASL_Object b) {
        if (a.type == UNDEF || b.type == UNDEF) {
            return UNDEF_C;
        }
        switch(a.type) {
        case BOOL:
            if (b.type == BOOL) {
                if (a.value.ival == b.value.ival) {
                    return TRUE_C;
                } else {
                    return FALSE_C;
                }
            } else {
                return FALSE_C;
            }
        case MAP:
            if (b.type == MAP) {
                puts("Warning: comparison of hashes currently is not implemented.");
                return UNDEF_C;
            }
            return FALSE_C;
        case LIST:
            /*if (b.type == LIST) {
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
            if (b.type == LIST) {
                puts("Warning: comparison of lists currently is not implemented.");
                return UNDEF_C;
            }
            return FALSE_C;
        case STR8:
            if (b.type == STR8) {
                if ((a.value.sval)->length != (b.value.sval)->length) {
                    return FALSE_C;
                } else {
                    int i = 0;
                    while (i < (a.value.sval)->length) {
                        if ((a.value.sval)->str[i] != (b.value.sval)->str[i]) {
                            return FALSE_C;
                        }
                        i++;
                    }
                    return TRUE_C;
                }
            }
            return FALSE_C;
        default:
            if (b.type == BOOL || b.type == MAP) {
                return FALSE_C;
            }
            int c;
            if (a.type == INT64 && b.type == INT64) {
                c = a.value.ival == b.value.ival;
            } else if (a.type == FLOAT64 && b.type == INT64) {
                c = a.value.dval == (double)b.value.ival;
            } else if (a.type == INT64 && b.type == FLOAT64) {
                c = (double)a.value.ival == b.value.dval;
            } else if (a.type == FLOAT64 && b.type == FLOAT64) {
                c = a.value.dval == b.value.dval;
            } else {
                printf("== and != not supported for operands of types %x and %x.\n", a.type, b.type);
                return UNDEF_C;
            }
            return (YASL_Object) {BOOL, c};
        }
}

int print(YASL_Object v) {
    int64_t i;
    switch (v.type) {
        case INT64:
            printf("%" PRId64 "", v.value.ival);
            //printf("int64: %" PRId64 "\n", v.value);
            break;
        case FLOAT64:
            printf("%f", *((double*)&v.value));
            //printf("float64: %f\n", *((double*)&v.value));
            break;
        case BOOL:
            if (v.value.ival == 0) printf("false");
            else printf("true");
            break;
        case UNDEF:
            printf("undef");
            break;
        case STR8:
            for (i = 0; i < (v.value.sval)->length; i++) {
                printf("%c", (v.value.sval)->str[i]);
            }
            break;
        /* case MAP:
            printf("<hash %" PRIx64 ">", v.value);
            break; */
        /* case LIST:
            //ls_print((List_t*)v.value);
            // printf("<list %" PRIx64 ">", v.value);
            break; */
        case FILEH:
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
        case FN_P:
            printf("<fn: %" PRIx64 ">", v.value.ival);
            break;
        case MN_P:
            printf("<mn: %" PRIx64 ">", v.value.ival);
            break;
        default:
            printf("Error, unknown type: %x", v.type);
            return -1;
    }
    return 0;
}
