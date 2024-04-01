#include "YASL_ByteBuffer.h"

#include <stdio.h>
#include <string.h>

#include "util/varint.h"
#include "common/debug.h"

YASL_ByteBuffer *YASL_ByteBuffer_new(const size_t size) {
	YASL_ByteBuffer *bb = (YASL_ByteBuffer *)malloc(sizeof(YASL_ByteBuffer));
	*bb = NEW_BB(size);
	return bb;
}

void YASL_ByteBuffer_del(YASL_ByteBuffer *const bb) {
	free(bb->items);
	free(bb);
}

void YASL_ByteBuffer_extend(YASL_ByteBuffer *const bb, const unsigned char *const bytes, const size_t bytes_len) {
	if (bb->size < bb->count + bytes_len)
		bb->items = (unsigned char *)realloc(bb->items, bb->size = (bb->count + bytes_len) * 2);
	memcpy(bb->items + bb->count, bytes, bytes_len);
	bb->count += bytes_len;
}

void YASL_ByteBuffer_add_vint(YASL_ByteBuffer *const bb, size_t val) {
	unsigned char buff[12];
	int len = vint_encode(val, buff);
	if (bb->size < bb->count + len) {
		bb->size = (bb->count + len) * 2;
		bb->items = (unsigned char *)realloc(bb->items, bb->size);
	}
	memcpy(bb->items + bb->count, buff, (size_t)len);
	bb->count += len;
}

void YASL_ByteBuffer_add_float(YASL_ByteBuffer *const bb, const yasl_float value) {
	if (bb->size < bb->count + sizeof(yasl_float)) bb->items = (unsigned char *)realloc(bb->items, bb->size =
			(bb->count + sizeof(yasl_float)) * 2);
	memcpy(bb->items + bb->count, &value, sizeof(yasl_float));
	bb->count += sizeof(yasl_float);
}

void YASL_ByteBuffer_add_int(YASL_ByteBuffer *const bb, const yasl_int value) {
	if (bb->size < bb->count + sizeof(yasl_int))
		bb->items = (unsigned char *)realloc(bb->items, bb->size = (bb->count + sizeof(yasl_int)) * 2);
	memcpy(bb->items + bb->count, &value, sizeof(yasl_int));
	bb->count += sizeof(yasl_int);
}


void YASL_ByteBuffer_rewrite_int_fast(YASL_ByteBuffer *const bb, const size_t index, const yasl_int value) {
	YASL_ASSERT(bb->count >= index + sizeof(yasl_int), "index is out of range.");
	memcpy(bb->items + index, &value, sizeof(yasl_int));
}
