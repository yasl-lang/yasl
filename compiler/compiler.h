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
	.globals = scope_new(NULL),\
	.stack = NULL,\
	.params = NULL,\
	.leftmost_pattern = true,\
	.seen_bindings = NEW_TABLE(),\
	.strings = YASL_Table_new(),\
	.buffer = YASL_ByteBuffer_new(16),\
	.header = YASL_ByteBuffer_new(24),\
	.code = YASL_ByteBuffer_new(16),\
	.lines = YASL_ByteBuffer_new(16),\
	.line = 0,\
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
	size_t num_upvals;
	signed char *upvals;
};

struct Compiler {
	struct Parser parser;
	struct Scope *globals;
	struct Scope *stack;
	struct Env *params;
	bool leftmost_pattern;
	struct YASL_Table seen_bindings;
	struct YASL_Table *strings;
	struct YASL_ByteBuffer *buffer;    // temporary buffer during code-gen
	struct YASL_ByteBuffer *header;    // header includes things like string constants
	struct YASL_ByteBuffer *code;      // bytecode generated
	struct YASL_ByteBuffer *lines;     // keeps track of current line number
	size_t line;
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
