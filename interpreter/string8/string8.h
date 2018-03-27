#pragma once

#include <inttypes.h>
#include <stdlib.h>

typedef struct {
    int64_t length;
    char* str;
} String_t;

static String_t* new_sized_string8(const int64_t base_size);
static String_t* new_sized_string8_from_mem(const int64_t base_size, char* str_mem);
static void del_string8(String_t* str8);
static int64_t string8_search(const String_t* haystack, const String_t* needle);
