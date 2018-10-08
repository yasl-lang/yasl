#pragma once

#include <inttypes.h>
#include "../ast/ast.h"
#include "../parser/parser.h"
#include "../bytebuffer/bytebuffer.h"
#include "../../opcode.h"
#include "../../functions.h"
#include "env/env.h"
#include "../../debug.h"

typedef struct {
    Parser *parser;
    Env_t *globals;
    Env_t *params;
    Env_t *locals;
    Hash_t *functions;
    int64_t offset;
    Hash_t *strings;
    ByteBuffer *buffer;
    ByteBuffer *header;
    ByteBuffer *code;
    int64_t *checkpoints;
    int64_t checkpoints_count;
    int64_t checkpoints_size;
    int status;
} Compiler;

Compiler *compiler_new(Parser *const parser);
void compiler_del(Compiler *compiler);
char *compile(Compiler *const compiler);