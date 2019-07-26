#pragma once

#include "yasl_conf.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    unsigned char *bytes;
    size_t size;
    size_t count;
} ByteBuffer;

ByteBuffer *bb_new(const size_t size);
void bb_del(ByteBuffer *const bb);

void bb_add_byte(ByteBuffer *const bb, const unsigned char byte);
void bb_append(ByteBuffer *const bb, const unsigned char *const bytes, const size_t bytes_len);
void bb_floatbytes8(ByteBuffer *const bb, const yasl_float value);
void bb_intbytes8(ByteBuffer *const bb, const yasl_int value);
void bb_rewrite_intbytes8(ByteBuffer *const bb, const size_t index, const yasl_int value);
