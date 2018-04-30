#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *bytes;
    int64_t size;
    int64_t count;
} ByteBuffer;

ByteBuffer *bb_new(int64_t size);
void bb_del(ByteBuffer *bb);

