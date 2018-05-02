#include "compiler.h"



Compiler *compiler_new(Parser* parser) {
    Compiler *compiler = malloc(sizeof(Compiler));
    compiler->globals = env_new(NULL);
    compiler->locals = env_new(NULL);
    env_decl_var(compiler->globals, "stdin", 5);
    env_decl_var(compiler->globals, "stdout", 6);
    env_decl_var(compiler->globals, "stderr", 6);
    compiler->parser = parser;
    compiler->buffer = bb_new(16);
    compiler->header = bb_new(16);
    compiler->header->count = 16;
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
    bb_del(compiler->code);
    parser_del(compiler->parser);
    free(compiler);
};

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
    memcpy(compiler->header->bytes, &compiler->header->count, sizeof(int64_t));
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

void visit_Print(Compiler* compiler, Node *node) {
    //printf("compiler->header->count is %d\n", compiler->header->count);
    visit(compiler, node->children[0]);
    char print_bytes[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};      // TODO: lookup in table
    bb_add_byte(compiler->buffer, BCALL_8);
    bb_append(compiler->buffer, print_bytes, 8);
}

void visit_BinOp(Compiler *compiler, Node *node) {
    //printf("compiler->header->count is %d\n", compiler->header->count);
    // TODO: make sure complicated bin ops are handled on their own.
    if (node->type == TOK_TBAR) {
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
    bb_intbytes8(compiler->buffer, env_get(compiler->globals, node->name, node->name_len));
}

void visit_String(Compiler* compiler, Node *node) {
    //printf("compiler->header->count is %d\n", compiler->header->count);
    // TODO: store string we've already seen.
    bb_add_byte(compiler->buffer, NEWSTR8);
    bb_intbytes8(compiler->buffer, node->name_len);
    bb_intbytes8(compiler->buffer, compiler->header->count);
    bb_append(compiler->header, node->name, node->name_len);
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
    case NODE_PRINT:
        puts("Visit Print");
        visit_Print(compiler, node);
        break;
    case NODE_BINOP:
        puts("Visit BinOp");
        visit_BinOp(compiler, node);
        break;
    case NODE_UNOP:
        puts("Visit UnOp");
        visit_UnOp(compiler, node);
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