#pragma once

#include "yasl_conf.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct YASL_ByteBuffer {
    unsigned char *bytes;
    size_t size;
    size_t count;
};

struct YASL_ByteBuffer *bb_new(const size_t size);
void bb_del(struct YASL_ByteBuffer *const bb);

void bb_add_byte(struct YASL_ByteBuffer *const bb, const unsigned char byte);
void bb_append(struct YASL_ByteBuffer *const bb, const unsigned char *const bytes, const size_t bytes_len);
void bb_floatbytes8(struct YASL_ByteBuffer *const bb, const yasl_float value);
void bb_intbytes8(struct YASL_ByteBuffer *const bb, const yasl_int value);
void bb_rewrite_intbytes8(struct YASL_ByteBuffer *const bb, const size_t index, const yasl_int value);
