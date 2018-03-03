#pragma once

#include "constant.h"
#include "../string8/string8.c"
#include <stdio.h>
#include <stdlib.h>
#define DVAL(v)  (*((double*)&v.value))
#define TRUE_C   ((Constant) {BOOL, 1})
#define FALSE_C  ((Constant) {BOOL, 0})
#define UNDEF_C  ((Constant) {UNDEF, 0})


Constant isequal(Constant a, Constant b) {
        if (a.type == UNDEF || b.type == UNDEF) {
            return UNDEF_C;
        }
        switch(a.type) {
        case BOOL:
            if (b.type == BOOL) {
                if (a.value == b.value) {
                    return TRUE_C;
                } else {
                    return FALSE_C;
                }
            } else {
                return FALSE_C;
            }
        case HASH:
            if (b.type == HASH) {
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
            // #define LEN_C(v)     (*((int64_t*)v.value))
        case STR8:
            //puts("str8");
            if (b.type == STR8) {
                if (((String_t*)a.value)->length != ((String_t*)b.value)->length) {
                    //puts("diff len");
                    return FALSE_C;
                } else {
                    int i = 0;
                    while (i < ((String_t*)a.value)->length) {
                        if (((String_t*)a.value)->str[i] != ((String_t*)b.value)->str[i]) {
                            //printf("a[%d], b[%d]: %x, %x\n", i, i, ((String_t*)a.value)->str[i], ((String_t*)b.value)->str[i]);
                            //puts("diff val at i");
                            return FALSE_C;
                        }
                        i++;
                    }
                    //puts("true");
                    return TRUE_C;
                }
            }
            //puts("b has wrong type");
            return FALSE_C;
        default:
            if (b.type == BOOL || b.type == HASH) {
                return FALSE_C;
            }
            int c;
            if (a.type == INT64 && b.type == INT64) {
                c = a.value == b.value;
            } else if (a.type == FLOAT64 && b.type == INT64) {
                c = (*((double*)&a.value)) == (double)b.value;
            } else if (a.type == INT64 && b.type == FLOAT64) {
                c = (double)a.value == (*((double*)&b.value));
            } else if (a.type == FLOAT64 && b.type == FLOAT64) {
                c = (*((double*)&a.value)) == (*((double*)&b.value));
            } else {
                printf("== and != not supported for operands of types %x and %x.\n", a.type, b.type);
                return;
            }
            return (Constant) {BOOL, c};
        }
}

int print(Constant v) {
    int i;
    switch (v.type) {
        case INT64:
            printf("int64: %" PRId64 "\n", v.value);
            break;
        case FLOAT64:
            printf("float64: %f\n", *((double*)&v.value));
            break;
        case BOOL:
            if (v.value == 0) printf("bool: false\n");
            else printf("bool: true\n");
            break;
        case UNDEF:
            printf("undef: undef\n");
            break;
        case STR8:
            printf("str: ");
            int64_t i;
            for (i = 0; i < ((String_t*)v.value)->length; i++) { // TODO: fix hardcoded 8
                //printf("%.*s\n", ((String_t*)v.value)->length, ((String_t*)v.value)->str);
                printf("%c", ((String_t*)v.value)->str[i]);
            }
            printf("\n");
            break;
        case HASH:
            printf("hash: <%" PRIx64 ">\n", v.value);
            break;;
        case LIST:
            printf("list: <%" PRIx64 ">\n", v.value);
            break;
        default:
            printf("Error, unknown type: %x\n", v.type);
            return -1;
    }
    return 0;
}
