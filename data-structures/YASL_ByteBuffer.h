#ifndef YASL_YASL_BYTEBUFFER_H_
#define YASL_YASL_BYTEBUFFER_H_

#include <stdlib.h>

#include "yasl_conf.h"

struct YASL_ByteBuffer {
	size_t size;
	size_t count;
	unsigned char *bytes;
};

struct YASL_ByteBuffer *YASL_ByteBuffer_new(const size_t size);
void YASL_ByteBuffer_del(struct YASL_ByteBuffer *const bb);

void YASL_ByteBuffer_extend(struct YASL_ByteBuffer *const bb, const unsigned char *const bytes, const size_t bytes_len);
void YASL_ByteBuffer_add_byte(struct YASL_ByteBuffer *const bb, const unsigned char byte);
void YASL_ByteBuffer_add_float(struct YASL_ByteBuffer *const bb, const yasl_float value);
void YASL_ByteBuffer_add_int(struct YASL_ByteBuffer *const bb, const yasl_int value);
/* Caller must check that index is within range */
void YASL_ByteBuffer_rewrite_int_fast(struct YASL_ByteBuffer *const bb, const size_t index, const yasl_int value);

#endif
