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

void bb_add_byte(ByteBuffer *bb, char byte);
void bb_append(ByteBuffer *bb, char *bytes, int64_t bytes_len);
void bb_floatbytes8(ByteBuffer *bb, double value);
void bb_intbytes8(ByteBuffer *bb, int64_t value);
void bb_rewrite_intbytes8(ByteBuffer *bb, int64_t index, int64_t value);
