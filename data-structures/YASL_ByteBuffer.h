#pragma once

#include <stdlib.h>

#include "yasl_conf.h"

struct YASL_ByteBuffer {
	size_t size;
	size_t count;
	unsigned char *bytes;
};

struct YASL_ByteBuffer *bb_new(const size_t size);
void bb_del(struct YASL_ByteBuffer *const bb);

void bb_extend(struct YASL_ByteBuffer *const bb, const unsigned char *const bytes, const size_t bytes_len);
void bb_add_byte(struct YASL_ByteBuffer *const bb, const unsigned char byte);
void bb_add_float(struct YASL_ByteBuffer *const bb, const yasl_float value);
void bb_add_int(struct YASL_ByteBuffer *const bb, const yasl_int value);
/* Caller must check that index is within range */
void bb_rewrite_int_fast(struct YASL_ByteBuffer *const bb, const size_t index, const yasl_int value);
