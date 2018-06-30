#include "YASL_string.h"
#include <string.h>

//#define STRLEN(s)
String_t* str_new_sized(const int64_t base_size) {
    String_t* str = malloc(sizeof(String_t));
    str->length = base_size;
    str->str = malloc(sizeof(char)*str->length);
    return str;
}

String_t* str_new_sized_from_mem(const int64_t base_size, char *str_mem) {
    String_t* str = malloc(sizeof(String_t));
    str->length = base_size;
    str->str = str_mem;
    return str;
}

//TODO: add new string constructor that takes address of string as second param.

void str_del(String_t *str8) {
    free(str8->str);
    free(str8);
}

int64_t str_find_index(const String_t *haystack, const String_t *needle) {
    // TODO: implement non-naive algorithm for string search.
    if (haystack->length < needle->length) return -1;
    int64_t i = 0;
    while (i < haystack->length) {
        if (!memcmp(haystack->str+i, needle->str, needle->length)) return i;
        i++;
    }
    return -1;
}
