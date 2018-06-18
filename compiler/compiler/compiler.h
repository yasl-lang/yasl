#pragma once

#include <inttypes.h>
#include "../ast/ast.h"
#include "../parser/parser.h"
#include "../bytebuffer/bytebuffer.h"
#include "../../opcode.h"
#include "../../methods.h"
#include "../../functions.h"
#include "env/env.h"
#include "../../debug.h"

typedef struct {
    Parser *parser;
    char *name;
    Env_t *globals;
    Env_t *locals;
    Hash_t *builtins;
    Hash_t *functions;
    Hash_t *functions_locals_len;
    int64_t offset;
    Hash_t *methods;
    Hash_t *strings;
    ByteBuffer *buffer;
    ByteBuffer *header;
    ByteBuffer *code;
    int64_t *checkpoints;
    int64_t checkpoints_count;
    int64_t checkpoints_size;
    char *current_function;
} Compiler;

Compiler *compiler_new(Parser *parser, char *name);
void compiler_del(Compiler *compiler);
void compile(Compiler *compiler);
void enter_scope(Compiler *compiler);
void exit_scope(Compiler *compiler);

void visit(Compiler *compiler, const Node *const node);
void visit_Block(Compiler *compiler, const Node *const node);
