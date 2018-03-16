#pragma once

#include <inttypes.h>

typedef struct {
    int64_t length;
    char* str;
} String_t;

String_t* new_sized_string8(const int64_t base_size);
String_t* new_sized_string8_from_mem(const int64_t base_size, char* str_mem);