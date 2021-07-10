#include "YASL_Buffer.h"

#include <stdlib.h>
#include <compiler/compiler.h>

#define DEF_BUFFER_POP(T) \
T BUFFER_POP(T)(BUFFER(T) *buffer) {\
	return buffer->items[buffer->count--];\
}

#define DEF_BUFFER_PUSH(T) \
void BUFFER_PUSH(T)(BUFFER(T) *buffer, T v) {\
	if (buffer->size <= buffer->count) {\
		buffer->size = buffer->count * 2;\
		buffer->items = (T *)realloc(buffer->items, sizeof(T) * buffer->size);\
	}\
	buffer->items[buffer->count++] = v;\
}

DEF_BUFFER_POP(size_t)
DEF_BUFFER_PUSH(size_t)

DEF_BUFFER_PUSH(byte)
