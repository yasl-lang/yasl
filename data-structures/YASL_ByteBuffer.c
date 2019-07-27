#include "YASL_ByteBuffer.h"

#include <stdio.h>
#include <string.h>

#include "debug.h"

struct YASL_ByteBuffer *bb_new(const size_t size) {
	struct YASL_ByteBuffer *bb = (struct YASL_ByteBuffer *)malloc(sizeof(struct YASL_ByteBuffer));
	bb->size = size;
	bb->bytes = (unsigned char *)malloc(sizeof(unsigned char) * bb->size);
	bb->count = 0;
	return bb;
}

void bb_del(struct YASL_ByteBuffer *const bb) {
	free(bb->bytes);
	free(bb);
}

void bb_extend(struct YASL_ByteBuffer *const bb, const unsigned char *const bytes, const size_t bytes_len) {
	if (bb->size < bb->count + bytes_len) bb->bytes = (unsigned char *)realloc(bb->bytes, bb->size = (bb->count + bytes_len) * 2);
	memcpy(bb->bytes + bb->count, bytes, bytes_len);
	bb->count += bytes_len;
}

void bb_add_byte(struct YASL_ByteBuffer  *const bb, const unsigned char byte) {
	if (bb->size <= bb->count) bb->bytes = (unsigned char *)realloc(bb->bytes, bb->size = bb->count * 2);
	bb->bytes[bb->count++] = byte;
}

void bb_add_float(struct YASL_ByteBuffer *const bb, const yasl_float value) {
	if (bb->size < bb->count + sizeof(yasl_float)) bb->bytes = (unsigned char *)realloc(bb->bytes, bb->size =
			(bb->count + sizeof(yasl_float)) * 2);
	memcpy(bb->bytes + bb->count, &value, sizeof(yasl_float));
	bb->count += sizeof(yasl_float);
}

void bb_add_int(struct YASL_ByteBuffer *const bb, const yasl_int value) {
	if (bb->size < bb->count + sizeof(yasl_int))
		bb->bytes = (unsigned char *)realloc(bb->bytes, bb->size = (bb->count + sizeof(yasl_int)) * 2);
	memcpy(bb->bytes + bb->count, &value, sizeof(yasl_int));
	bb->count += sizeof(yasl_int);
}


void bb_rewrite_int_fast(struct YASL_ByteBuffer *const bb, const size_t index, const yasl_int value) {
	YASL_ASSERT(bb->size < index + sizeof(yasl_int), "index is out of range.");
	memcpy(bb->bytes + index, &value, sizeof(yasl_int));
}
