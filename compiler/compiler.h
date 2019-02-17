#pragma once

#include <inttypes.h>
#include "compiler/ast.h"
#include "parser.h"
#include "bytebuffer/bytebuffer.h"
#include "opcode.h"
#include "env.h"
#include "debug.h"

#define NEW_COMPILER(fp)\
((struct Compiler) {\
	.parser = (NEW_PARSER(fp)),\
	.globals = env_new(NULL),\
	.params = NULL,\
	.strings = table_new(),\
	.buffer = bb_new(16),\
	.header = bb_new(16),\
	.status = YASL_SUCCESS,\
	.checkpoints_size = 4,\
	.checkpoints = malloc(sizeof(size_t) * 4),\
	.checkpoints_count = 0,\
	.code = bb_new(16)\
})

struct Compiler {
    Parser parser;
    Env_t *globals;
    Env_t *params;
    struct Table *strings;
    ByteBuffer *buffer;
    ByteBuffer *header;
    ByteBuffer *code;
    size_t *checkpoints;
    size_t checkpoints_count;
    size_t checkpoints_size;
    int status;
};

struct Compiler *compiler_new(FILE *fp);
struct Compiler *compiler_new_bb(char *buf, int len);
void compiler_cleanup(struct Compiler *compiler);
unsigned char *compile(struct Compiler *const compiler);
unsigned char *compile_REPL(struct Compiler *const compiler);