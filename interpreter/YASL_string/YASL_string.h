#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <interpreter/refcount/refcount.h>

typedef struct {
    unsigned char *str;
    int64_t start;
    int64_t end;
    int from_mem;
    RefCount* rc;
} String_t;

// typedef String_t YASL_str;


int64_t yasl_string_len(const String_t *const str);
unsigned char *copy_char_buffer(const int64_t size, const unsigned char *const ptr);
String_t* str_new_sized(const int64_t base_size, unsigned char *ptr);
String_t* str_new_sized_from_mem(const int64_t start, const int64_t end, unsigned char *mem);
void str_del_data(String_t *str);
void str_del_rc(String_t *str);
void str_del(String_t *str);
int64_t str_find_index(const String_t *haystack, const String_t *needle);
