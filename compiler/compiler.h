#ifndef YASL_COMPILER_H
#define YASL_COMPILER_H

#include <inttypes.h>

#include "data-structures/YASL_ByteBuffer.h"
#include "debug.h"
#include "env.h"
#include "parser.h"

#define NEW_COMPILER(fp) new_compiler(fp)

struct Compiler {
    struct Parser parser;
    struct Env *globals;
    struct Env *stack;
    struct Env *params;
    size_t num_locals;
    struct YASL_Table *strings;
    struct YASL_ByteBuffer *buffer;
    struct YASL_ByteBuffer *header;
    struct YASL_ByteBuffer *code;
    size_t *checkpoints;
    size_t checkpoints_count;
    size_t checkpoints_size;
    int status;
    int64_t num;
};

struct Compiler new_compiler(struct LEXINPUT *lp) {
	struct Compiler c = ((struct Compiler) {\
	.globals = env_new(NULL),\
	.stack = NULL,\
	.params = NULL,\
        .num_locals = 0,	\
	.strings = YASL_Table_new(),\
	.buffer = YASL_ByteBuffer_new(16),\
	.header = YASL_ByteBuffer_new(16),\
	.code = YASL_ByteBuffer_new(16),\
        .checkpoints = (size_t *)malloc(sizeof(size_t) * 4),\
	.checkpoints_count = 0,\
	.checkpoints_size = 4,\
	.status = 0,\
	.num = 0,\
  });
	c.parser = new_parser(lp);
	return c;
}

struct Compiler *compiler_new(FILE *const fp);
struct Compiler *compiler_new_bb(const char *const buf, const size_t len);
void compiler_cleanup(struct Compiler *const compiler);
unsigned char *compile(struct Compiler *const compiler);
unsigned char *compile_REPL(struct Compiler *const compiler);

#endif
