#ifndef YASL_YASL_BYTEBUFFER_H_
#define YASL_YASL_BYTEBUFFER_H_

#include <stdlib.h>

#include "YASL_Buffer.h"
#include "yasl_conf.h"

DECL_BUFFER(byte)

typedef BUFFER(byte) YASL_ByteBuffer;

#define NEW_BB(s) ((YASL_ByteBuffer){\
	.size = (s),\
	.count = 0,\
	.items = (byte *)malloc(s)\
})

YASL_ByteBuffer *YASL_ByteBuffer_new(const size_t size);
void YASL_ByteBuffer_del(YASL_ByteBuffer *const bb);

void YASL_ByteBuffer_extend(YASL_ByteBuffer *const bb, const byte *const bytes, const size_t bytes_len);
#define YASL_ByteBuffer_add_byte BUFFER_PUSH(byte)
// void YASL_ByteBuffer_add_byte(YASL_ByteBuffer *const buffer, const byte v);
void YASL_ByteBuffer_add_vint(YASL_ByteBuffer *const bb, size_t val);
void YASL_ByteBuffer_add_float(YASL_ByteBuffer *const bb, const yasl_float value);
void YASL_ByteBuffer_add_int(YASL_ByteBuffer *const bb, const yasl_int value);
/* Caller must check that index is within range */
void YASL_ByteBuffer_rewrite_int_fast(YASL_ByteBuffer *const bb, const size_t index, const yasl_int value);

#endif
