#pragma once

typedef struct {
    char *ptr;
    int *count;
} rcptr;


void rcptr_inc(rcptr ptr);

void rcptr_dec(rcptr ptr);

rcptr rcptr_new(char *ptr);

rcptr rcptr_copy(rcptr ptr);