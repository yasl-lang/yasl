#include "YASL_string.h"
#include <string.h>

int64_t yasl_string_len(const String_t *const str) {
    return str->end - str->start;
}

String_t* str_new_sized(const int64_t base_size, unsigned char *ptr) {
    String_t* str = malloc(sizeof(String_t));
    str->start = 0;
    str->end = base_size;
    str->str = ptr;
    str->from_mem = 0;
    return str;
}

String_t* str_new_sized_from_mem(const int64_t start, const int64_t end, unsigned char *mem) {
    String_t* str = malloc(sizeof(String_t));
    str->start = start;
    str->end = end;
    str->str = mem;
    str->from_mem = 1;
    return str;
}

//TODO: add new string constructor that takes address of string as second param.

void str_del(String_t *str8) {
    if(!str8->from_mem) free(str8->str);
    free(str8);
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
