#ifndef YASL_COMPILER_H_
#define YASL_COMPILER_H_

#include <inttypes.h>

#include "data-structures/YASL_ByteBuffer.h"
#include "debug.h"
#include "env.h"
#include "parser.h"

#define NEW_SIZEBUFFER(s)\
	((struct SizeBuffer){\
		.count = 0,\
		.size = (s),\
		.items = (size_t *)malloc(sizeof(size_t)*(s)),\
	})

#define NEW_COMPILER(fp)\
  ((struct Compiler) {\
    .parser = NEW_PARSER(fp),\
    .globals = env_new(NULL),\
    .stack = NULL,\
    .params = NULL,\
    .outer = NULL,\
    .strings = YASL_Table_new(),\
    .buffer = YASL_ByteBuffer_new(16),\
    .header = YASL_ByteBuffer_new(16),\
    .code = YASL_ByteBuffer_new(16),\
    .checkpoints = NEW_SIZEBUFFER(4),\
    .locals = (struct Frame *)malloc(sizeof(struct Frame) * 4),\
    .locals_count = 0,\
    .locals_size = 4,\
    .status = YASL_SUCCESS,\
    .num = 0,\
  })

struct SizeBuffer {
	size_t count;
	size_t size;
	size_t *items;
};

struct Frame {
	size_t num_locals;
	size_t num_upvals;
	signed char *upvals;
};

struct Compiler {
	struct Parser parser;
	struct Env *globals;
	struct Env *stack;
	struct Env *params;
	struct Env *outer;
	struct YASL_Table *strings;
	struct YASL_ByteBuffer *buffer;
	struct YASL_ByteBuffer *header;
	struct YASL_ByteBuffer *code;
	struct SizeBuffer checkpoints;
	struct Frame *locals;
	size_t locals_count;
	size_t locals_size;
	int status;
	int64_t num;
};

struct Compiler *compiler_new(FILE *const fp);
struct Compiler *compiler_new_bb(const char *const buf, const size_t len);
void compiler_cleanup(struct Compiler *const compiler);
unsigned char *compile(struct Compiler *const compiler);
unsigned char *compile_REPL(struct Compiler *const compiler);

#endif
