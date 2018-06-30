#pragma once

#include <inttypes.h>
#include <stdlib.h>

typedef struct {
    int64_t length;
    char* str;
} String_t;

// typedef String_t YASL_str;

String_t* str_new_sized(const int64_t base_size);
String_t* str_new_sized_from_mem(const int64_t base_size, char *str_mem);
void str_del(String_t *str8);
int64_t str_find_index(const String_t *haystack, const String_t *needle);
