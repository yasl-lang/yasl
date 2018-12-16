#pragma once

#include <inttypes.h>
#include <stdlib.h>

#include "refcount.h"

typedef struct {
    struct RC* rc;      // RC MUST BE THE FIRST MEMBER OF THIS STRUCT. DO NOT REARRANGE.
    char *str;
    size_t start;
    int64_t end;
    int from_mem;
} String_t;

// typedef String_t YASL_str;


int64_t yasl_string_len(const String_t *const str);
int64_t yasl_string_cmp(const String_t *const left, const String_t *const right);
char *copy_char_buffer(const int64_t size, const char *const ptr);
String_t* str_new_sized(int64_t base_size, char *ptr);
String_t* str_new_sized_from_mem(int64_t start, int64_t end, char *mem);
void str_del_data(String_t *str);
void str_del_rc(String_t *str);
void str_del(String_t *str);
int64_t str_find_index(const String_t *haystack, const String_t *needle);
