#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <yasl_include.h>

#include "interpreter/refcount.h"

typedef struct {
    struct RC* rc;      // NOTE: RC MUST BE THE FIRST MEMBER OF THIS STRUCT. DO NOT REARRANGE.
    char *str;
    size_t start;
    int64_t end;
    bool on_heap;
} String_t;


int64_t yasl_string_len(const String_t *const str);
int64_t yasl_string_cmp(const String_t *const left, const String_t *const right);
char *copy_char_buffer(const int64_t size, const char *const ptr);
String_t* str_new_sized(int64_t base_size, char *ptr);
String_t *str_new_substring(const int64_t start, const int64_t end, const String_t *const string);
String_t* str_new_sized_heap(const int64_t start, const int64_t end, char *const mem);
void str_del_data(String_t *const str);
void str_del_rc(String_t *const str);
void str_del(String_t *const str);
int64_t str_find_index(const String_t *const haystack, const String_t *const needle);
