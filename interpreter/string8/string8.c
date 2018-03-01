#pragma once

#include "string8.h"

static String_t* new_sized_string8(const int64_t base_size) {
    String_t* str = malloc(sizeof(String_t));
    str->length = base_size;
    str->str = malloc(sizeof(char)*str->length);
    return str;
}

void del_string8(String_t* str) {
    free(str->str);
    free(str);
}