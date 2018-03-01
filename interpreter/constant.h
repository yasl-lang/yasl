#pragma once

#include "string8/string8.c"
#define FALSEY(v)  (v.type == UNDEF || (v.type == BOOL && v.value == 0) || (v.type == STR8 && ((String_t*)v.value)->length == 0))  // returns true iff v is a falsey value

enum types {
    UNDEF   = 0x00,
    FLOAT64 = 0x13,
    INT64   = 0x1B,
    BOOL    = 0x20,
    STR8    = 0x32,
    LIST    = 0x44,
    HASH    = 0x48,
};

typedef struct {
    char type;
    int64_t value;
} Constant;

typedef struct {
    char type;
    double value;
} FloatConstant;

Constant isequal(Constant a, Constant b);
int print(Constant a);