#include "YASL_string.h"
#include <string.h>
#include <stdio.h>

int64_t yasl_string_len(const String_t *const str) {
    return str->end - str->start;
}

unsigned char *copy_char_buffer(const int64_t size, const unsigned char *const ptr) {
    unsigned char *tmp = malloc(size);
    memcpy(tmp, ptr, size);
    return tmp;
}

String_t* str_new_sized(const int64_t base_size, unsigned char *ptr) {
    String_t* str = malloc(sizeof(String_t));
    str->start = 0;
    str->end = base_size;
    str->str = ptr;
    str->from_mem = 0;
    str->rc = rc_new();
    return str;
}

String_t* str_new_sized_from_mem(const int64_t start, const int64_t end, unsigned char *mem) {
    String_t* str = malloc(sizeof(String_t));
    str->start = start;
    str->end = end;
    str->str = mem;
    str->from_mem = 1;
    str->rc = rc_new();
    return str;
}

//TODO: add new string constructor that takes address of string as second param.

void str_del_data(String_t *str) {
    if(!str->from_mem) free(str->str);
}

void str_del_rc(String_t *str) {
    rc_del(str->rc);
    free(str);

}

void str_del(String_t *str) {
    if(!str->from_mem) free(str->str);
    rc_del(str->rc);
    free(str);
}


int64_t str_find_index(const String_t *haystack, const String_t *needle) {
    // TODO: implement non-naive algorithm for string search.
    if (yasl_string_len(haystack) < yasl_string_len(needle)) return -1;
    int64_t i = 0;
    while (i < yasl_string_len(haystack)) {
        if (!memcmp(haystack->str + i, needle->str, yasl_string_len(needle))) return i;
        i++;
    }
    return -1;
}
