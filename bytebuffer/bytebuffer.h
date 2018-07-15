#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    unsigned char *bytes;
    int64_t size;
    int64_t count;
} ByteBuffer;

ByteBuffer *bb_new(int64_t size);
void bb_del(ByteBuffer *bb);

void bb_add_byte(ByteBuffer *const bb, const unsigned char byte);
void bb_append(ByteBuffer *const bb, const unsigned char *const bytes, const int64_t bytes_len);
void bb_floatbytes8(ByteBuffer *const bb, const double value);
void bb_intbytes8(ByteBuffer *const bb, const int64_t value);
void bb_rewrite_intbytes8(ByteBuffer *const bb, const int64_t index, const int64_t value);
