#pragma once

#include <inttypes.h>
#include "compiler/ast.h"
#include "parser.h"
#include "data-structures/YASL_ByteBuffer.h"
#include "opcode.h"
#include "env.h"
#include "debug.h"

#define NEW_COMPILER(fp)\
  ((struct Compiler) {\
  	.parser = NEW_PARSER(fp),\
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
	.status = YASL_SUCCESS,\
	.num = 0\
  })

struct Compiler {
    struct Parser parser;
    Env_t *globals;
    Env_t *stack;
    Env_t *params;
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

struct Compiler *compiler_new(FILE *const fp);
struct Compiler *compiler_new_bb(const char *const buf, const size_t len);
void compiler_cleanup(struct Compiler *const compiler);
unsigned char *compile(struct Compiler *const compiler);
unsigned char *compile_REPL(struct Compiler *const compiler);
