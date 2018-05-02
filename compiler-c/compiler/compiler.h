#pragma once

#include <inttypes.h>
#include "../ast/ast.h"
#include "../parser/parser.h"
#include "../bytebuffer/bytebuffer.h"
#include "../../opcode.h"
#include "../../methods.h"
#include "../env/env.h"

/*
class Compiler(NodeVisitor):
    def __init__(self):
        self.globals = Env()
        self.globals.decl_var("stdin")
        self.globals.decl_var("stdout")
        self.globals.decl_var("stderr")
        self.locals = Env()
        self.code = []
        self.checkpoints = []
        self.header = intbytes_8(8) + intbytes_8(0)
        self.buffer = []
        self.fns = {}
        self.current_fn = None
        self.offset = 0
        self.strs = {}
*/
typedef struct {
    Parser *parser;
    /*
    Env *globals; // make sure to declare stdin, stdout, stderr
    Env *locals;
    char *header;
    int64_t header_len;
    char *code;
    int64_t code_len;
    */
    Env_t *globals;
    Env_t *locals;
    ByteBuffer *buffer;
    ByteBuffer *header;
    ByteBuffer *code;
    /*int64_t *checkpoints;
    STable *functions;
    char *curfn;
    int64_t offset;
    STable *strings; */
} Compiler;

Compiler *compiler_new(Parser *parser);
void compiler_del(Compiler *compiler);
void compile(Compiler *compiler);


void visit_Print(Compiler *compiler, Node *node);
void visit_UnOp(Compiler *compiler, Node *node);
void visit_Var(Compiler *compiler, Node *node);
void visit_Undef(Compiler *compiler, Node *node);
void visit_String(Compiler *compiler, Node *node);
void visit(Compiler *compiler, Node* node);
