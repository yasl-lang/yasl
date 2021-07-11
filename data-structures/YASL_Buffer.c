#include "YASL_Buffer.h"

#include <stdlib.h>
#include <compiler/compiler.h>

#define DEF_BUFFER_INIT(T) \
void BUFFER_INIT(T)(BUFFER(T) *buffer, size_t size) {\
	*buffer = (BUFFER(T)) {\
		.size = size,\
		.count = 0,\
		.items = (T *)malloc(sizeof(T) * size)\
	};\
}

#define DEF_BUFFER_CLEANUP(T) \
void BUFFER_CLEANUP(T)(BUFFER(T) *buffer) {\
	free(buffer->items);\
}

#define DEF_BUFFER_COPY(T) \
BUFFER(T) BUFFER_COPY(T)(BUFFER(T) *buffer) {\
	BUFFER(T) copy;\
	BUFFER_INIT(T)(&copy, buffer->size);\
	memcpy(copy.items, buffer->items, sizeof(T) * buffer->count);\
	copy.count = buffer->count;\
	return copy;\
}

#define DEF_BUFFER_POP(T) \
T BUFFER_POP(T)(BUFFER(T) *buffer) {\
	return buffer->items[--buffer->count];\
}

#define DEF_BUFFER_PUSH(T) \
void BUFFER_PUSH(T)(BUFFER(T) *buffer, T v) {\
	if (buffer->size <= buffer->count) {\
		buffer->size *= 2;\
		buffer->items = (T *)realloc(buffer->items, sizeof(T) * buffer->size);\
	}\
	buffer->items[buffer->count++] = v;\
}

DEF_BUFFER_POP(size_t)
DEF_BUFFER_PUSH(size_t)

DEF_BUFFER_PUSH(byte)

DEF_BUFFER_INIT(ptr)
DEF_BUFFER_CLEANUP(ptr)
DEF_BUFFER_COPY(ptr)
DEF_BUFFER_PUSH(ptr)