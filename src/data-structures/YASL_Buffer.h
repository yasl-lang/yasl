#ifndef YASL_YASL_BUFFER_H_
#define YASL_YASL_BUFFER_H_

#include <stdlib.h>

typedef unsigned char byte;
typedef void *ptr;

#define MANGLE_NAME(T, F) T##_Buffer_##F

#define BUFFER(T) struct T##_Buffer
#define BUFFER_INIT(T) MANGLE_NAME(T, Init)
#define BUFFER_CLEANUP(T) MANGLE_NAME(T, CleanUp)
#define BUFFER_COPY(T) MANGLE_NAME(T, Copy)
#define BUFFER_PUSH(T) MANGLE_NAME(T, Push)
#define BUFFER_POP(T) MANGLE_NAME(T, Pop)

#define DECL_BUFFER(T) struct T##_Buffer {\
	size_t size;\
	size_t count;\
	T *items;\
};\
\
void BUFFER_INIT(T)(BUFFER(T) *, size_t);\
void BUFFER_CLEANUP(T)(BUFFER(T) *);\
BUFFER(T) BUFFER_COPY(T)(BUFFER(T) *);\
void BUFFER_PUSH(T)(BUFFER(T) *, T);\
T BUFFER_POP(T)(BUFFER(T) *);

DECL_BUFFER(ptr);

#endif
