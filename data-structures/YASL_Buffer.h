#ifndef YASL_YASL_BUFFER_H_
#define YASL_YASL_BUFFER_H_

typedef unsigned char byte;

#define MANGLE_NAME(T, F) T##_Buffer_##F

#define BUFFER(T) struct T##_Buffer
#define BUFFER_PUSH(T) MANGLE_NAME(T, Push)
#define BUFFER_POP(T) MANGLE_NAME(T, Pop)

#define DECL_BUFFER(T) struct T##_Buffer {\
	size_t size;\
	size_t count;\
	T *items;\
};\
\
void BUFFER_PUSH(T)(BUFFER(T) *, T);\
T BUFFER_POP(T)(BUFFER(T) *);

#endif
