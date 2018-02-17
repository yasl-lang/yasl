#pragma once

#include "constant.h"
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
                puts("comparison of hashes currently is not implemented.");
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
                puts("comparison of lists currently is not implemented.");
                return UNDEF_C;
            }
            return FALSE_C;
            // #define LEN_C(v)     (*((int64_t*)v.value))
        case STR:
            if (b.type == STR) {
                if (*((int64_t*)a.value) != *((int64_t*)b.value)) {
                    return FALSE_C;
                } else {
                    int i = sizeof(int64_t);
                    while (i < *((int64_t*)a.value) + sizeof(int64_t)) {
                        if (*((char*)(a.value + i)) != *((char*)(b.value + i))) {
                            return FALSE_C;
                        }
                        i++;
                    }
                    return TRUE_C;
                }
            }
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