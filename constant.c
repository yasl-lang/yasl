#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char type;
    int64_t value;
} Constant;

typedef struct {
    char type;
    double value;
} FloatConstant;