#include "compiler.h"
#define break_checkpoint(compiler)    (compiler->checkpoints[compiler->checkpoints_count-1])
#define continue_checkpoint(compiler) (compiler->checkpoints[compiler->checkpoints_count-2])


Compiler *compiler_new(Parser* parser) {
    Compiler *compiler = malloc(sizeof(Compiler));
    compiler->globals = env_new(NULL);
    compiler->locals = env_new(NULL);
    env_decl_var(compiler->globals, "stdin", 5);
    env_decl_var(compiler->globals, "stdout", 6);
    env_decl_var(compiler->globals, "stderr", 6);
    compiler->strings = new_hash();
    compiler->parser = parser;
    compiler->buffer = bb_new(16);
    compiler->header = bb_new(16);
    compiler->header->count = 16;
    compiler->checkpoints_size = 4;
    compiler->checkpoints = malloc(sizeof(int64_t)*compiler->checkpoints_size);
    compiler->checkpoints_count = 0;
    //printf("compiler->header->count is %d\n", compiler->header->count);
    compiler->code   = bb_new(16);
    return compiler;
};

void compiler_del(Compiler *compiler) {
    env_del(compiler->globals);
    env_del(compiler->locals);
    //puts("deleting buffer");
    bb_del(compiler->buffer);
    //puts("deleting header");
    bb_del(compiler->header);
    //puts("deleting code");
    free(compiler->checkpoints);
    bb_del(compiler->code);
    parser_del(compiler->parser);
    free(compiler);
};

void enter_scope(Compiler *compiler) {
    /*if self.current_fn is not None:
    self.locals = Env(self.locals)
    else:
    self.globals = Env(self.globals)    */
    // TODO: deal with locals
    compiler->globals = env_new(compiler->globals);
}

void exit_scope(Compiler *compiler) {
    /*
     *         if self.current_fn is not None:
            self.locals = self.locals.parent
        else:
            self.globals = self.globals.parent
     */
    compiler->globals = compiler->globals->parent;
    // TODO: deal with locals
    // TODO: deal with memory leaks
}

void add_checkpoint(Compiler *compiler, int64_t cp) {
    if (compiler->checkpoints_count >= compiler->checkpoints_size)
        compiler->checkpoints = realloc(compiler->checkpoints, compiler->checkpoints_size *= 2);
    compiler->checkpoints[compiler->checkpoints_count++] = cp;
}

void rm_checkpoint(Compiler *compiler) {
    compiler->checkpoints_count--;
}

void compile(Compiler *compiler) {
    Node *node;
    gettok(compiler->parser->lex);
    while (!peof(compiler->parser)) {
            if (peof(compiler->parser)) break;
            node = parse(compiler->parser);
            eattok(compiler->parser, TOK_SEMI);
            puts("about to print");
            printf("eaten semi. type: %s, value: %s\n", YASL_TOKEN_NAMES[compiler->parser->lex->type], compiler->parser->lex->value);
            //puts("parsed");
            //printf("compiler->header->count is %d\n", compiler->header->count);
            puts("about to visit");
            visit(compiler, node);
            puts("visited");
            //puts("visited");
            //printf("compiler->header->count is %d\n", compiler->header->count);
            bb_append(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
            //puts("appended");
            //printf("compiler->header->count is %d\n", compiler->header->count);
            compiler->buffer->count = 0;
            //printf("compiler->header->count is %d\n", compiler->header->count);
            node_del(node);
            //printf("compiler->header->count is %d\n", compiler->header->count);
            //puts("deleted node");
    }
    //puts("ready to calculate header");
    //printf("compiler->header->count is %d\n", compiler->header->count);
    bb_rewrite_intbytes8(compiler->header, 0, compiler->header->count);
    //memcpy(compiler->header->bytes, &compiler->header->count, sizeof(int64_t));
    //puts("calculated header");
    int i = 0;
    for (i = 0; i < compiler->header->count; i++) {
        printf("%02x\n", compiler->header->bytes[i]);// && 0xFF);
    }
    puts("entry point");
    for (i = 0; i < compiler->code->count; i++) {
        printf("%02x\n", compiler->code->bytes[i]);// & 0xFF);
    }
    printf("%02x\n", HALT);
    FILE *fp = fopen("source.yb", "wb");
    if (!fp) exit(EXIT_FAILURE);
    fwrite(compiler->header->bytes, 1, compiler->header->count, fp);
    fwrite(compiler->code->bytes, 1, compiler->code->count, fp);
    fputc(HALT, fp);
    fclose(fp);
}

void visit_ExprStmt(Compiler *compiler, Node *node) {
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, POP);
}

void visit_Block(Compiler *compiler, Node *node) {
    int i;
    for (i = 0; i < node->children_len; i++) {
        visit(compiler, node->children[i]);
    }
}

void visit_While(Compiler *compiler, Node *node) {
    /*
     *         cond = self.visit(node.cond)
        self.checkpoints.append(len(self.code))
        self.checkpoints.append(len(self.code)+len(cond))
        self.enter_scope()
        body = []
        for stmt in node.body:
            body.extend(self.visit(stmt))
        self.exit_scope()
        self.checkpoints.pop()
        self.checkpoints.pop()
        cond.extend([BRF_8] + intbytes_8(len(body)+9))
        body.extend([BR_8] + intbytes_8(-(len(body)+9+len(cond))))
        return cond + body
     */
    int64_t index_start = compiler->code->count + compiler->buffer->count;
    add_checkpoint(compiler, index_start);
    // printf("index start = %d\n", index_start);
    visit(compiler, node->children[0]);
    add_checkpoint(compiler, compiler->code->count + compiler->buffer->count);
    bb_add_byte(compiler->buffer, BRF_8);
    int64_t index_second = compiler->buffer->count;
    bb_intbytes8(compiler->buffer, 0);
    // TODO: checkpoints.
    enter_scope(compiler);
    visit(compiler, node->children[1]);
    bb_add_byte(compiler->buffer, GOTO);
    bb_intbytes8(compiler->buffer, index_start);
    bb_rewrite_intbytes8(compiler->buffer, index_second, compiler->buffer->count - index_second - 8);
    exit_scope(compiler);

    rm_checkpoint(compiler);
    rm_checkpoint(compiler);
    // TODO: checkpoints

}

void visit_Break(Compiler *compiler, Node *node) {
    if (compiler->checkpoints_count == 0) {
        puts("SyntaxError: break outside of loop.");
        exit(EXIT_FAILURE);
    }
    bb_add_byte(compiler->buffer, BCONST_F);
    bb_add_byte(compiler->buffer, GOTO);
    bb_intbytes8(compiler->buffer, break_checkpoint(compiler));
}

void visit_Continue(Compiler *compiler, Node *node) {
    if (compiler->checkpoints_count == 0) {
        puts("SyntaxError: continue outside of loop.");
        exit(EXIT_FAILURE);
    }
    bb_add_byte(compiler->buffer, GOTO);
    bb_intbytes8(compiler->buffer, continue_checkpoint(compiler));
    //bb_intbytes8(compiler->buffer, continue_checkpoint(compiler));
}

void visit_Print(Compiler* compiler, Node *node) {
    //printf("compiler->header->count is %d\n", compiler->header->count);
    visit(compiler, node->children[0]);
    char print_bytes[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};      // TODO: lookup in table
    bb_add_byte(compiler->buffer, BCALL_8);
    bb_append(compiler->buffer, print_bytes, 8);
}

/*
 *     def visit_Decl(self, node):
        result = []
        for i in range(len(node.left)):
            var = node.left[i]
            val = node.right[i]
            if self.current_fn is not None:
                if var.value not in self.locals.vars:
                    self.locals[var.value] = len(self.locals.vars) + 1 - self.offset
                result = result + self.visit(val) + [LSTORE_1, self.locals[var.value]]
                continue
            if var.value not in self.globals.vars:
                self.globals.decl_var(var.value)
            right = self.visit(val)
            result = result + right + [GSTORE_1, self.globals[var.value]]
        return result
 */
void visit_Let(Compiler *compiler, Node *node) {
    if (!env_contains(compiler->globals, node->name, node->name_len)) {
        env_decl_var(compiler->globals, node->name, node->name_len);
    }
    if (node->children != NULL) visit(compiler, node->children[0]);
    else bb_add_byte(compiler->buffer, NCONST);
    // TODO: handle locals
    bb_add_byte(compiler->buffer, GSTORE_1);
    bb_add_byte(compiler->buffer, env_get(compiler->globals, node->name, node->name_len));
}

void visit_TriOp(Compiler *compiler, Node *node) {
    /*cond = self.visit(node.cond)
    left = self.visit(node.left)
    right = self.visit(node.right)
    left = left + [BR_8] + intbytes_8(len(right))
    return cond + [BRF_8] + intbytes_8(len(left)) + left + right */
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, BRF_8);
    int64_t index_l = compiler->buffer->count;
    bb_intbytes8(compiler->buffer, 0);
    visit(compiler, node->children[1]);
    bb_add_byte(compiler->buffer, BR_8);
    int64_t index_r = compiler->buffer->count;
    bb_intbytes8(compiler->buffer, 0);
    bb_rewrite_intbytes8(compiler->buffer, index_l, compiler->buffer->count-index_l-8);
    visit(compiler, node->children[2]);
    bb_rewrite_intbytes8(compiler->buffer, index_r, compiler->buffer->count-index_r-8);
    return;
}

void visit_BinOp(Compiler *compiler, Node *node) {
    //printf("compiler->header->count is %d\n", compiler->header->count);
    // TODO: make sure complicated bin ops are handled on their own.
    if (node->type == TOK_DQMARK) {
        // return left + [DUP, BRN_8] + intbytes_8(len(right)+1) + [POP] + right
        visit(compiler, node->children[0]);
        bb_add_byte(compiler->buffer, DUP);
        bb_add_byte(compiler->buffer, BRN_8);
        int64_t index = compiler->buffer->count;
        bb_intbytes8(compiler->buffer, 0);
        bb_add_byte(compiler->buffer, POP);
        visit(compiler, node->children[1]);
        bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count-index-8);
        return;
    } else if (node->type == TOK_OR) {
        visit(compiler, node->children[0]);
        bb_add_byte(compiler->buffer, DUP);
        bb_add_byte(compiler->buffer, BRT_8);
        int64_t index = compiler->buffer->count;
        bb_intbytes8(compiler->buffer, 0);
        bb_add_byte(compiler->buffer, POP);
        visit(compiler, node->children[1]);
        bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count-index-8);
        return;
    } else if (node->type == TOK_AND) {
        visit(compiler, node->children[0]);
        bb_add_byte(compiler->buffer, DUP);
        bb_add_byte(compiler->buffer, BRF_8);
        int64_t index = compiler->buffer->count;
        bb_intbytes8(compiler->buffer, 0);
        bb_add_byte(compiler->buffer, POP);
        visit(compiler, node->children[1]);
        bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count-index-8);
        return;
    } else if (node->type == TOK_TBAR) {
            /* return left + [MCALL_8] + intbytes_8(METHODS["tostr"]) + right + [MCALL_8] + intbytes_8(METHODS["tostr"]) \
                + [HARD_CNCT] */
            visit(compiler, node->children[0]);
            bb_add_byte(compiler->buffer, MCALL_8);
            bb_intbytes8(compiler->buffer, TOSTR);
            visit(compiler, node->children[1]);
            bb_add_byte(compiler->buffer, MCALL_8);
            bb_intbytes8(compiler->buffer, TOSTR);
            bb_add_byte(compiler->buffer, HARD_CNCT);
            return;
    }
    visit(compiler, node->children[0]);
    visit(compiler, node->children[1]);
    switch(node->type) {
        case TOK_BAR:
            bb_add_byte(compiler->buffer, BOR);
            break;
        case TOK_TILDE:
            bb_add_byte(compiler->buffer, BXOR);
            break;
        case TOK_AMP:
            bb_add_byte(compiler->buffer, BAND);
            break;
        case TOK_DEQ:
            bb_add_byte(compiler->buffer, EQ);
            break;
        case TOK_TEQ:
            bb_add_byte(compiler->buffer, ID);
            break;
        case TOK_BANGEQ:
            bb_add_byte(compiler->buffer, EQ);
            bb_add_byte(compiler->buffer, NOT);
            break;
        case TOK_BANGDEQ:
            bb_add_byte(compiler->buffer, ID);
            bb_add_byte(compiler->buffer, NOT);
            break;
        case TOK_GT:
            bb_add_byte(compiler->buffer, GT);
            break;
        case TOK_GTEQ:
            bb_add_byte(compiler->buffer, GE);
            break;
        case TOK_LT:
            bb_add_byte(compiler->buffer, GE);
            bb_add_byte(compiler->buffer, NOT);
            break;
        case TOK_LTEQ:
            bb_add_byte(compiler->buffer, GT);
            bb_add_byte(compiler->buffer, NOT);
            break;
        case TOK_DBAR:
            bb_add_byte(compiler->buffer, CNCT);
            break;
        case TOK_DGT:
            bb_add_byte(compiler->buffer, BRSHIFT);
            break;
        case TOK_DLT:
            bb_add_byte(compiler->buffer, BLSHIFT);
            break;
        case TOK_PLUS:
            bb_add_byte(compiler->buffer, ADD);
            break;
        case TOK_MINUS:
            bb_add_byte(compiler->buffer, SUB);
            break;
        case TOK_STAR:
            bb_add_byte(compiler->buffer, MUL);
            break;
        case TOK_SLASH:
            bb_add_byte(compiler->buffer, FDIV);
            break;
        case TOK_DSLASH:
            bb_add_byte(compiler->buffer, IDIV);
            break;
        case TOK_MOD:
            bb_add_byte(compiler->buffer, MOD);
            break;
        case TOK_CARET:
            bb_add_byte(compiler->buffer, EXP);
            break;
        default:
            puts("error in visit_BinOp");
            exit(EXIT_FAILURE);
    }
}

void visit_UnOp(Compiler *compiler, Node *node) {
    visit(compiler, node->children[0]);
    switch(node->type) {
        case TOK_PLUS:
            bb_add_byte(compiler->buffer, NOP);
            break;
        case TOK_MINUS:
            bb_add_byte(compiler->buffer, NEG);
            break;
        case TOK_BANG:
            bb_add_byte(compiler->buffer, NOT);
            break;
        case TOK_TILDE:
            bb_add_byte(compiler->buffer, BNOT);
            break;
        case TOK_HASH:
            bb_add_byte(compiler->buffer, LEN);
            break;
        default:
            puts("error in visit_UnOp");
            exit(EXIT_FAILURE);
    }
}

void visit_Assign(Compiler *compiler, Node *node) {
    if (!env_contains(compiler->globals, node->name, node->name_len) &&
        !env_contains(compiler->locals, node->name, node->name_len)) {
        printf("unknown variable: %s\n", node->name);
    }
    // TODO: handle locals
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, DUP);
    bb_add_byte(compiler->buffer, GSTORE_1);
    bb_add_byte(compiler->buffer, env_get(compiler->globals, node->name, node->name_len));
}

void visit_Var(Compiler *compiler, Node *node) {
    printf("%s is global: %d\n", node->name, env_contains(compiler->globals, node->name, node->name_len));
    printf("%s is local: %d\n", node->name, env_contains(compiler->locals, node->name, node->name_len));
    if (!env_contains(compiler->globals, node->name, node->name_len) &&
        !env_contains(compiler->locals, node->name, node->name_len)) {
        printf("unknown variable: %s\n", node->name);
    }
    // TODO: handle case with functions
    /*return [GLOAD_1, self.globals[node.value]] */
    bb_add_byte(compiler->buffer, GLOAD_1);
    bb_add_byte(compiler->buffer, env_get(compiler->globals, node->name, node->name_len));
}

void visit_String(Compiler* compiler, Node *node) {
    //printf("compiler->header->count is %d\n", compiler->header->count);
    // TODO: deal with memory leaks introduced here.
    String_t *string = malloc(sizeof(String_t));
    string->length = node->name_len;
    string->str = malloc(string->length);
    memcpy(string->str, node->name, string->length);
    Constant key = (Constant) { .value = (int64_t)string, .type = STR8 };

    Constant *value = ht_search(compiler->strings, key);
    if (value == NULL) {
        puts("caching");
        ht_insert(compiler->strings, key, (Constant) { .type = INT64, .value = compiler->header->count});
        bb_append(compiler->header, node->name, node->name_len);
    }

    value = ht_search(compiler->strings, key);

    bb_add_byte(compiler->buffer, NEWSTR8);
    bb_intbytes8(compiler->buffer, node->name_len);
    bb_intbytes8(compiler->buffer, value->value); //compiler->header->count);
    // bb_append(compiler->header, node->name, node->name_len);
    //printf("compiler->header->count is %d\n", compiler->header->count);
}

void visit_Undef(Compiler *compiler, Node *node) {
    bb_add_byte(compiler->buffer, NCONST);
}

void visit_Float(Compiler *compiler, Node *node) {
    bb_add_byte(compiler->buffer, DCONST);
    bb_floatbytes8(compiler->buffer, strtod(node->name, (char**)NULL));
}

void visit_Integer(Compiler *compiler, Node *node) {
    bb_add_byte(compiler->buffer, ICONST);
    if (node->name_len < 2) {
        bb_intbytes8(compiler->buffer, (int64_t)strtoll(node->name, (char**)NULL, 10));
        return;
    }
    switch(node->name[1]) {
        case 'x':
            bb_intbytes8(compiler->buffer, (int64_t)strtoll(node->name+2, (char**)NULL, 16));
            break;
        case 'b':
            bb_intbytes8(compiler->buffer, (int64_t)strtoll(node->name+2, (char**)NULL, 2));
            break;
        case 'o':
            bb_intbytes8(compiler->buffer, (int64_t)strtoll(node->name+2, (char**)NULL, 8));
            break;
        default:
            bb_intbytes8(compiler->buffer, (int64_t)strtoll(node->name, (char**)NULL, 10));
            break;
    }
}

void visit_Boolean(Compiler *compiler, Node *node) {
    if (!memcmp(node->name, "true", node->name_len)) {
        bb_add_byte(compiler->buffer, BCONST_T);
        return;
    } else if (!memcmp(node->name, "false", node->name_len)) {
        bb_add_byte(compiler->buffer, BCONST_F);
        return;
    }
}

void visit(Compiler* compiler, Node* node) {
    //printf("compiler->header->count is %d\n", compiler->header->count);
    //printf("node type is %x\n", node->nodetype);
    //puts("about to switch");
    switch(node->nodetype) {
    case NODE_EXPRSTMT:
        puts("Visit ExprStmt");
        visit_ExprStmt(compiler, node);
        break;
    case NODE_BLOCK:
        puts("Visit Block");
        visit_Block(compiler, node);
        break;
    case NODE_WHILE:
        puts("Visit While");
        visit_While(compiler, node);
        break;
    case NODE_BREAK:
        puts("Visit Break");
        visit_Break(compiler, node);
        break;
    case NODE_CONT:
        puts("Visit Continue");
        visit_Continue(compiler, node);
        break;
    case NODE_PRINT:
        puts("Visit Print");
        visit_Print(compiler, node);
        break;
    case NODE_LET:
        puts("Visit Let");
        visit_Let(compiler, node);
        break;
    case NODE_TRIOP:
        puts("Visit TriOp");
        visit_TriOp(compiler, node);
        break;
    case NODE_BINOP:
        puts("Visit BinOp");
        visit_BinOp(compiler, node);
        break;
    case NODE_UNOP:
        puts("Visit UnOp");
        visit_UnOp(compiler, node);
        break;
    case NODE_ASSIGN:
        puts("Visit Assign");
        visit_Assign(compiler, node);
        break;
    case NODE_VAR:
        puts("Visit Var");
        visit_Var(compiler, node);
        break;
    case NODE_UNDEF:
        puts("Visit Undef");
        visit_Undef(compiler, node);
        break;
    case NODE_FLOAT64:
        puts("Visit Float");
        visit_Float(compiler, node);
        break;
    case NODE_INT64:
        puts("Visit Integer");
        visit_Integer(compiler, node);
        break;
    case NODE_BOOL:
        puts("Visit Boolean");
        visit_Boolean(compiler, node);
        break;
    case NODE_STR:
        puts("Visit String");
        visit_String(compiler, node);
        break;
    default:
        printf("%d\n", node->nodetype);
        puts("unknown node type");
        exit(1);
    }
}