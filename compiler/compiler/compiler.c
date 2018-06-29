#include <interpreter/YASL_Object/YASL_Object.h>
#include <interpreter/YASL_string/YASL_string.h>
#include <bytebuffer/bytebuffer.h>
#include <metadata.h>
#include "compiler.h"
#define break_checkpoint(compiler)    (compiler->checkpoints[compiler->checkpoints_count-1])
#define continue_checkpoint(compiler) (compiler->checkpoints[compiler->checkpoints_count-2])


static Compiler *compiler_tables_init(const Compiler *const compiler) {
    ht_insert_string_int(compiler->builtins, "open", strlen("open"), F_OPEN);
    ht_insert_string_int(compiler->builtins, "popen", strlen("popen"), F_POPEN);
    ht_insert_string_int(compiler->builtins, "input", strlen("input"), F_INPUT);

    ht_insert_string_int(compiler->methods, "tofloat64", strlen("tofloat64"), M_TOFLOAT64);
    ht_insert_string_int(compiler->methods, "toint64", strlen("toint64"), M_TOINT64);
    ht_insert_string_int(compiler->methods, "tobool", strlen("tobool"), M_TOBOOL);
    ht_insert_string_int(compiler->methods, "tostr", strlen("tostr"), M_TOSTR);

    ht_insert_string_int(compiler->methods, "upcase", strlen("upcase"), M_UPCASE);
    ht_insert_string_int(compiler->methods, "downcase", strlen("downcase"), M_DOWNCASE);
    ht_insert_string_int(compiler->methods, "isalnum", strlen("isalnum"), M_ISALNUM);
    ht_insert_string_int(compiler->methods, "isal", strlen("isal"), M_ISAL);
    ht_insert_string_int(compiler->methods, "isnum", strlen("isnum"), M_ISNUM);
    ht_insert_string_int(compiler->methods, "isspace", strlen("isspace"), M_ISSPACE);
    ht_insert_string_int(compiler->methods, "startswith", strlen("startswith"), M_STARTSWITH);
    ht_insert_string_int(compiler->methods, "endswith", strlen("endswith"), M_ENDSWITH);
    ht_insert_string_int(compiler->methods, "search", strlen("search"), M_SEARCH);
    ht_insert_string_int(compiler->methods, "split", strlen("split"), M_SPLIT);
    ht_insert_string_int(compiler->methods, "ltrim", strlen("ltrim"), M_LTRIM);
    ht_insert_string_int(compiler->methods, "rtrim", strlen("rtrim"), M_RTRIM);
    ht_insert_string_int(compiler->methods, "trim", strlen("trim"), M_TRIM);

    ht_insert_string_int(compiler->methods, "append", strlen("append"), M_APPEND);

    ht_insert_string_int(compiler->methods, "keys", strlen("keys"), M_KEYS);
    ht_insert_string_int(compiler->methods, "values", strlen("values"), M_VALUES);
    ht_insert_string_int(compiler->methods, "clone", strlen("clone"), M_CLONE);

    ht_insert_string_int(compiler->methods, "close", strlen("close"), M_CLOSE);
    ht_insert_string_int(compiler->methods, "pclose", strlen("pclose"), M_PCLOSE);
    ht_insert_string_int(compiler->methods, "read", strlen("read"), M_READ);
    ht_insert_string_int(compiler->methods, "write", strlen("write"), M_WRITE);
    ht_insert_string_int(compiler->methods, "readline", strlen("readline"), M_READLINE);

    ht_insert_string_int(compiler->methods, "__set", strlen("__set"), M___SET);
    ht_insert_string_int(compiler->methods, "__get", strlen("__get"), M___GET);
}

Compiler *compiler_new(Parser *const parser, char *const name) {
    Compiler *compiler = malloc(sizeof(Compiler));

    compiler->globals = env_new(NULL);
    compiler->locals = env_new(NULL);
    env_decl_var(compiler->globals, "stdin", strlen("stdin"));
    env_decl_var(compiler->globals, "stdout", strlen("stdout"));
    env_decl_var(compiler->globals, "stderr", strlen("stderr"));

    compiler->builtins = new_hash();
    compiler->methods = new_hash();
    compiler_tables_init(compiler);

    compiler->functions = new_hash();
    compiler->functions_locals_len = new_hash();
    compiler->offset = 0;
    compiler->strings = new_hash();
    compiler->parser = parser;
    compiler->name = name;
    compiler->buffer = bb_new(16);
    compiler->header = bb_new(16);
    compiler->header->count = 16;
    /*bb_add_byte(compiler->header, 'Y');
    bb_add_byte(compiler->header, 'Y');
    bb_add_byte(compiler->header, 'Y');
    bb_add_byte(compiler->header, 'Y');
    bb_add_byte(compiler->header, 'Y');
    bb_add_byte(compiler->header, 'Y');
    bb_add_byte(compiler->header, 'Y');
    bb_add_byte(compiler->header, 'Y');
    char magic_number[YASL_MAG_NUM_SIZE] = "YASL";
    magic_number[4] = YASL_COMPILER;
    magic_number[5] = YASL_MAJOR_VERSION;
    magic_number[6] = YASL_MINOR_VERSION;
    magic_number[7] = YASL_PATCH; */
    compiler->checkpoints_size = 4;
    compiler->checkpoints = malloc(sizeof(int64_t)*compiler->checkpoints_size);
    compiler->checkpoints_count = 0;
    compiler->code = bb_new(16);
    return compiler;
};

void compiler_tables_del(Compiler *compiler) {
    del_hash_string_int(compiler->strings);
    del_hash_string_int(compiler->functions);
    del_hash_string_int(compiler->functions_locals_len);
    del_hash_string_int(compiler->methods);
    del_hash_string_int(compiler->builtins);
}

static void compiler_buffers_del(const Compiler *const compiler) {
    bb_del(compiler->buffer);
    bb_del(compiler->header);
    bb_del(compiler->code);
}

void compiler_del(Compiler *compiler) {
    compiler_tables_del(compiler);
    env_del(compiler->globals);
    env_del(compiler->locals);
    parser_del(compiler->parser);
    compiler_buffers_del(compiler);
    free(compiler->checkpoints);
    free(compiler);
};

static void enter_scope(Compiler *const compiler) {
    /*if self.current_fn is not None:
    self.locals = Env(self.locals)
    else:
    self.globals = Env(self.globals)    */
    // TODO: deal with locals
    compiler->globals = env_new(compiler->globals);
}

static void exit_scope(Compiler *const compiler) {
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

static void add_checkpoint(Compiler *const compiler, const int64_t cp) {
    if (compiler->checkpoints_count >= compiler->checkpoints_size)
        compiler->checkpoints = realloc(compiler->checkpoints, compiler->checkpoints_size *= 2);
    compiler->checkpoints[compiler->checkpoints_count++] = cp;
}

static void rm_checkpoint(Compiler *compiler) {
    compiler->checkpoints_count--;
}

static void visit(const Compiler *const compiler, const Node *const node);
static void visit_Block(const Compiler *const compiler, const Node *const node);

void compile(const Compiler *const compiler) {
    Node *node;
    gettok(compiler->parser->lex);
    while (!peof(compiler->parser)) {
            if (peof(compiler->parser)) break;
            node = parse(compiler->parser);
            eattok(compiler->parser, T_SEMI);
            visit(compiler, node);
            bb_append(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
            compiler->buffer->count = 0;
            node_del(node);
    }
    bb_rewrite_intbytes8(compiler->header, 0, compiler->header->count);

    int i = 0;
    /*YASL_DEBUG_LOG("%s\n", "magic number");
    for (i = 0; i < 7; i++) {
        YASL_DEBUG_LOG("%02x\n", magic_number[i]);
    } */
    YASL_DEBUG_LOG("%s\n", "header");
    for (i = 0; i < compiler->header->count; i++) {
        YASL_DEBUG_LOG("%02x\n", compiler->header->bytes[i]);
    }
    YASL_DEBUG_LOG("%s\n", "entry point");
    for (i = 0; i < compiler->code->count; i++) {
        YASL_DEBUG_LOG("%02x\n", compiler->code->bytes[i]);
    }
    YASL_DEBUG_LOG("%02x\n", HALT);
    FILE *fp = fopen(compiler->name, "wb");
    if (!fp) exit(EXIT_FAILURE);

    //fwrite(magic_number, 1, YASL_MAG_NUM_SIZE, fp);
    fwrite(compiler->header->bytes, 1, compiler->header->count, fp);
    fwrite(compiler->code->bytes, 1, compiler->code->count, fp);
    fputc(HALT, fp);
    fclose(fp);
}

static void visit_ExprStmt(const Compiler *const compiler, const Node *const node) {
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, POP);
}

static void visit_FunctionDecl(Compiler *const compiler, const Node *const node) {
    if (compiler->current_function != NULL) {
        puts("Illegal function declaration outside global scope.");
        exit(EXIT_FAILURE);
    }

    if (ht_search_string_int(compiler->functions, node->name, node->name_len) != NULL) {
        puts("Illegal redeclaration of function.");
        exit(EXIT_FAILURE);
    }

    if (ht_search_string_int(compiler->builtins, node->name, node->name_len) != NULL) {
        puts("Illegal redeclaration of builtin function.");
        exit(EXIT_FAILURE);
    }

    compiler->current_function =  node->name;

    // start logic for function, now that we are sure it's legal to do so, and have set up.

    // use offset to compute offsets for locals, in other functions.
    compiler->offset = node->children[0]->children_len;
    //YASL_DEBUG_LOG("compiler->offset is: %d\n", compiler->offset);

    compiler->locals = env_new(compiler->locals);

    int64_t i;
    for (i = node->children[0]->children_len - 1; i >= 0; i--) {
        ht_insert_string_int(compiler->locals->vars, node->children[0]->children[i]->name,
                             node->children[0]->children[i]->name_len, 255 - compiler->locals->vars->count - 1);
    }

    ht_insert_string_int(compiler->functions_locals_len, node->name, node->name_len, compiler->locals->vars->count);

    visit_Block(compiler, node->children[1]);

    ht_insert_string_int(compiler->functions, node->name, node->name_len, compiler->header->count);

    //YASL_DEBUG_LOG("tried to insert function, result was %d.\n", ht_search_string_int(compiler->functions, node->name, node->name_len) != NULL);

    bb_append(compiler->header, compiler->buffer->bytes, compiler->buffer->count);
    bb_add_byte(compiler->header, NCONST);
    bb_add_byte(compiler->header, RET);

    // zero buffer length to ensure t
    compiler->buffer->count = 0;

    // clean up, i.e. delete local env.

    compiler->current_function = NULL;

    Env_t *tmp = compiler->locals->parent;

    for (i = 0; i < compiler->locals->vars->size; i++) {
        Item_t* item = compiler->locals->vars->items[i];
        if (item != NULL) {
            del_string8(item->key->value.sval);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    free(compiler->locals->vars->items);
    free(compiler->locals->vars);
    free(compiler->locals);

    compiler->locals = tmp;

}

static void visit_Call(const Compiler *const compiler, const Node *const node) {
    YASL_TRACE_LOG("Visit Call: %s\n", node->name);
    // TODO: error handling on number of arguments.
    /*
    def visit_FunctionCall(self, node):
        result = []
        for expr in node.params:
            result = result + self.visit(expr)
        if node.value in BUILTINS:
            return result + [BCALL_8] + intbytes_8(BUILTINS[node.value])
        return result + [CALL_8, self.fns[node.token.value]["params"]] + \
                         intbytes_8(self.fns[node.token.value]["addr"]) + \
                         [self.fns[node.token.value]["locals"]]
     */
    //YASL_DEBUG_LOG("visiting call %s\n", node->name);
    visit_Block(compiler, node->children[0]);
    //YASL_TRACE_LOG("Visiting Block with %d children in reverse.\n", node->children_len);
    //int i;
    //for (i = node->children_len - 1; i >= 0; i--) {
    //    visit(compiler, node->children[i]);
    //}

    if (ht_search_string_int(compiler->builtins, node->name, node->name_len)) {
        bb_add_byte(compiler->buffer, BCALL_8);
        bb_intbytes8(compiler->buffer, ht_search_string_int(compiler->builtins, node->name, node->name_len)->value.ival);
    } else {
        if (NULL == ht_search_string_int(compiler->functions, node->name, node->name_len)) {
            printf("Undefined function: %s\n", node->name);
            exit(EXIT_FAILURE);
        }

        //visit_Block(compiler, node->children[0]);

        bb_add_byte(compiler->buffer, CALL_8);

        bb_add_byte(compiler->buffer, node->children[0]->children_len);

        bb_intbytes8(compiler->buffer, ht_search_string_int(compiler->functions, node->name, node->name_len)->value.ival);

        bb_add_byte(compiler->buffer, ht_search_string_int(compiler->functions_locals_len, node->name, node->name_len)->value.ival);


    }
}

static void visit_Return(const Compiler *const compiler, const Node *const node) {
    // deal with recursive calls.
    if (node->nodetype == N_CALL && !strcmp(compiler->current_function, node->name)) {
        visit_Block(compiler, node->children[0]);

        bb_add_byte(compiler->buffer, RCALL_8);
        bb_add_byte(compiler->buffer, node->children[0]->children_len);
        bb_intbytes8(compiler->buffer, ht_search_string_int(compiler->functions, node->name, node->name_len)->value.ival);
        bb_add_byte(compiler->buffer, ht_search_string_int(compiler->functions_locals_len, node->name, node->name_len)->value.ival);

        return;
    }

    // default case.
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, RET);
}

static void visit_Set(const Compiler *const compiler, const Node *const node) {
    // TODO: fix order here by changing VM
    visit(compiler, node->children[1]);
    visit(compiler, node->children[2]);
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, MCALL_8);
    bb_intbytes8(compiler->buffer, M___SET);
}

static void visit_Get(const Compiler *const compiler, const Node *const node) {
    visit(compiler, node->children[1]);
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, MCALL_8);
    bb_intbytes8(compiler->buffer, M___GET);
}

static void visit_Block(const Compiler *const compiler, const Node *const node) {
    YASL_TRACE_LOG("Visiting Block with %d children.\n", node->children_len);
    int i;
    for (i = 0; i < node->children_len; i++) {
        visit(compiler, node->children[i]);
    }
}

static void visit_While(const Compiler *const compiler, const Node *const node) {
    int64_t index_start = compiler->code->count + compiler->buffer->count;
    add_checkpoint(compiler, index_start);
    visit(compiler, node->children[0]);
    add_checkpoint(compiler, compiler->code->count + compiler->buffer->count);
    bb_add_byte(compiler->buffer, BRF_8);
    int64_t index_second = compiler->buffer->count;
    bb_intbytes8(compiler->buffer, 0);
    enter_scope(compiler);
    visit(compiler, node->children[1]);
    bb_add_byte(compiler->buffer, GOTO);
    bb_intbytes8(compiler->buffer, index_start);
    bb_rewrite_intbytes8(compiler->buffer, index_second, compiler->buffer->count - index_second - 8);
    exit_scope(compiler);

    rm_checkpoint(compiler);
    rm_checkpoint(compiler);
}

static void visit_Break(const Compiler *const compiler, const Node *const node) {
    if (compiler->checkpoints_count == 0) {
        puts("SyntaxError: break outside of loop.");
        exit(EXIT_FAILURE);
    }
    bb_add_byte(compiler->buffer, BCONST_F);
    bb_add_byte(compiler->buffer, GOTO);
    bb_intbytes8(compiler->buffer, break_checkpoint(compiler));
}

static void visit_Continue(const Compiler *const compiler, const Node *const node) {
    if (compiler->checkpoints_count == 0) {
        puts("SyntaxError: continue outside of loop.");
        exit(EXIT_FAILURE);
    }
    bb_add_byte(compiler->buffer, GOTO);
    bb_intbytes8(compiler->buffer, continue_checkpoint(compiler));
}

static void visit_If(const Compiler *const compiler, const Node *const node) {
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, BRF_8);
    int64_t index_then = compiler->buffer->count;
    bb_intbytes8(compiler->buffer, 0);
    enter_scope(compiler);
    visit(compiler, node->children[1]);
    exit_scope(compiler);
    int64_t index_else = 0;
    if (node->children[2] != NULL) {
        bb_add_byte(compiler->buffer, BR_8);
        index_else = compiler->buffer->count;
        bb_intbytes8(compiler->buffer, 0);
    }
    bb_rewrite_intbytes8(compiler->buffer, index_then, compiler->buffer->count-index_then-8);
    if (node->children[2] != NULL) {
        enter_scope(compiler);
        visit(compiler, node->children[2]);
        exit_scope(compiler);
        bb_rewrite_intbytes8(compiler->buffer, index_else, compiler->buffer->count-index_else-8);
    }
}

static void visit_Print(const Compiler *const compiler, const Node *const node) {
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, BCALL_8);
    bb_intbytes8(compiler->buffer, F_PRINT);
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
static void visit_Let(const Compiler *const compiler, const Node *const node) {
    if (NULL != compiler->current_function) {
        if (!env_contains(compiler->locals, node->name, node->name_len)) {
            YASL_DEBUG_LOG("inserting local var at index: %d\n", compiler->locals->vars->count + 1 - compiler->offset);
            ht_insert_string_int(compiler->locals->vars, node->name, node->name_len,
                                 compiler->locals->vars->count + 1 - compiler->offset);
        }
        visit(compiler, node->children[0]);
        bb_add_byte(compiler->buffer, LSTORE_1);
        bb_add_byte(compiler->buffer, ht_search_string_int(compiler->locals->vars, node->name, node->name_len)->value.ival);
        return;
    }

    if (!env_contains(compiler->globals, node->name, node->name_len)) {
        env_decl_var(compiler->globals, node->name, node->name_len);
    }
    if (node->children != NULL) visit(compiler, node->children[0]);
    else bb_add_byte(compiler->buffer, NCONST);

    bb_add_byte(compiler->buffer, GSTORE_1);
    bb_add_byte(compiler->buffer, env_get(compiler->globals, node->name, node->name_len));
}

static void visit_TriOp(const Compiler *const compiler, const Node *const node) {
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
}

static void visit_BinOp(const Compiler *const compiler, const Node *const node) {
    // complicated bin ops are handled on their own.
    if (node->type == T_DQMARK) {     // ?? operator
        visit(compiler, node->children[0]);
        bb_add_byte(compiler->buffer, DUP);
        bb_add_byte(compiler->buffer, BRN_8);
        int64_t index = compiler->buffer->count;
        bb_intbytes8(compiler->buffer, 0);
        bb_add_byte(compiler->buffer, POP);
        visit(compiler, node->children[1]);
        bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count-index-8);
        return;
    } else if (node->type == T_OR) {  // or operator
        visit(compiler, node->children[0]);
        bb_add_byte(compiler->buffer, DUP);
        bb_add_byte(compiler->buffer, BRT_8);
        int64_t index = compiler->buffer->count;
        bb_intbytes8(compiler->buffer, 0);
        bb_add_byte(compiler->buffer, POP);
        visit(compiler, node->children[1]);
        bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count-index-8);
        return;
    } else if (node->type == T_AND) {   // and operator
        visit(compiler, node->children[0]);
        bb_add_byte(compiler->buffer, DUP);
        bb_add_byte(compiler->buffer, BRF_8);
        int64_t index = compiler->buffer->count;
        bb_intbytes8(compiler->buffer, 0);
        bb_add_byte(compiler->buffer, POP);
        visit(compiler, node->children[1]);
        bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count-index-8);
        return;
    } else if (node->type == T_TBAR) {  // ||| operator
            /* return left + [MCALL_8] + intbytes_8(METHODS["tostr"]) + right + [MCALL_8] + intbytes_8(METHODS["tostr"]) \
                + [HARD_CNCT] */
            visit(compiler, node->children[0]);
            bb_add_byte(compiler->buffer, MCALL_8);
            bb_intbytes8(compiler->buffer, M_TOSTR);
            visit(compiler, node->children[1]);
            bb_add_byte(compiler->buffer, MCALL_8);
            bb_intbytes8(compiler->buffer, M_TOSTR);
            bb_add_byte(compiler->buffer, HARD_CNCT);
            return;
    }
    // all other operators follow the same pattern of visiting one child then the other.
    visit(compiler, node->children[0]);
    visit(compiler, node->children[1]);
    switch(node->type) {
        case T_BAR:
            bb_add_byte(compiler->buffer, BOR);
            break;
        case T_CARET:
            bb_add_byte(compiler->buffer, BXOR);
            break;
        case T_AMP:
            bb_add_byte(compiler->buffer, BAND);
            break;
        case T_DEQ:
            bb_add_byte(compiler->buffer, EQ);
            break;
        case T_TEQ:
            bb_add_byte(compiler->buffer, ID);
            break;
        case T_BANGEQ:
            bb_add_byte(compiler->buffer, EQ);
            bb_add_byte(compiler->buffer, NOT);
            break;
        case T_BANGDEQ:
            bb_add_byte(compiler->buffer, ID);
            bb_add_byte(compiler->buffer, NOT);
            break;
        case T_GT:
            bb_add_byte(compiler->buffer, GT);
            break;
        case T_GTEQ:
            bb_add_byte(compiler->buffer, GE);
            break;
        case T_LT:
            bb_add_byte(compiler->buffer, GE);
            bb_add_byte(compiler->buffer, NOT);
            break;
        case T_LTEQ:
            bb_add_byte(compiler->buffer, GT);
            bb_add_byte(compiler->buffer, NOT);
            break;
        case T_DBAR:
            bb_add_byte(compiler->buffer, CNCT);
            break;
        case T_DGT:
            bb_add_byte(compiler->buffer, BRSHIFT);
            break;
        case T_DLT:
            bb_add_byte(compiler->buffer, BLSHIFT);
            break;
        case T_PLUS:
            bb_add_byte(compiler->buffer, ADD);
            break;
        case T_MINUS:
            bb_add_byte(compiler->buffer, SUB);
            break;
        case T_STAR:
            bb_add_byte(compiler->buffer, MUL);
            break;
        case T_SLASH:
            bb_add_byte(compiler->buffer, FDIV);
            break;
        case T_DSLASH:
            bb_add_byte(compiler->buffer, IDIV);
            break;
        case T_MOD:
            bb_add_byte(compiler->buffer, MOD);
            break;
        case T_DSTAR:
            bb_add_byte(compiler->buffer, EXP);
            break;
        default:
            puts("error in visit_BinOp");
            exit(EXIT_FAILURE);
    }
}

static void visit_UnOp(const Compiler *const compiler, const Node *const node) {
    visit(compiler, node->children[0]);
    switch(node->type) {
        case T_PLUS:
            bb_add_byte(compiler->buffer, NOP);
            break;
        case T_MINUS:
            bb_add_byte(compiler->buffer, NEG);
            break;
        case T_BANG:
            bb_add_byte(compiler->buffer, NOT);
            break;
        case T_CARET:
            bb_add_byte(compiler->buffer, BNOT);
            break;
        case T_HASH:
            bb_add_byte(compiler->buffer, LEN);
            break;
        default:
            puts("error in visit_UnOp");
            exit(EXIT_FAILURE);
    }
}

static void visit_Assign(const Compiler *const compiler, const Node *const node) {
    if (!env_contains(compiler->globals, node->name, node->name_len) &&
        !env_contains(compiler->locals, node->name, node->name_len)) {
        printf("unknown variable: %s\n", node->name);
    }
    // TODO: handle locals
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, DUP);
    bb_add_byte(compiler->buffer, GSTORE_1);
    bb_add_byte(compiler->buffer, env_get(compiler->globals, node->name, node->name_len)); // TODO: handle size
}

static void visit_Var(const Compiler *const compiler, const Node *const node) {
    YASL_TRACE_LOG("%s is global: ", node->name);
    YASL_TRACE_LOG("%d\n", env_contains(compiler->globals, node->name, node->name_len));
    YASL_TRACE_LOG("%s is local: ", node->name);
    YASL_TRACE_LOG("%d\n", env_contains(compiler->locals, node->name, node->name_len));
    if (!env_contains(compiler->globals, node->name, node->name_len) &&
        !env_contains(compiler->locals, node->name, node->name_len)) {
        printf("unknown variable: %s\n", node->name);
        exit(EXIT_FAILURE);
    }

    if (compiler->current_function != NULL && env_contains(compiler->locals, node->name, node->name_len)) {
        bb_add_byte(compiler->buffer, LLOAD_1);
        bb_add_byte(compiler->buffer, env_get(compiler->locals, node->name, node->name_len)); // TODO: handle size
        return;
    }

    bb_add_byte(compiler->buffer, GLOAD_1);
    bb_add_byte(compiler->buffer, env_get(compiler->globals, node->name, node->name_len));  // TODO: handle size
}

static void visit_Undef(const Compiler *const compiler, const Node *const node) {
    bb_add_byte(compiler->buffer, NCONST);
}

static void visit_Float(const Compiler *const compiler, const Node *const node) {
    YASL_TRACE_LOG("float64: %s\n", node->name);
    bb_add_byte(compiler->buffer, DCONST);
    bb_floatbytes8(compiler->buffer, strtod(node->name, (char**)NULL));
}

static void visit_Integer(const Compiler *const compiler, const Node *const node) {
    bb_add_byte(compiler->buffer, ICONST);
    YASL_TRACE_LOG("int64: %s\n", node->name);
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

static void visit_Boolean(const Compiler *const compiler, const Node *const node) {
    if (!memcmp(node->name, "true", node->name_len)) {
        bb_add_byte(compiler->buffer, BCONST_T);
        return;
    } else if (!memcmp(node->name, "false", node->name_len)) {
        bb_add_byte(compiler->buffer, BCONST_F);
        return;
    }
}

static void visit_String(const Compiler *const compiler, const Node *const node) {
    YASL_Object *value = ht_search_string_int(compiler->strings, node->name, node->name_len);
    if (value == NULL) {
        YASL_DEBUG_LOG("%s\n", "caching string");
        ht_insert_string_int(compiler->strings, node->name, node->name_len, compiler->header->count);
        bb_intbytes8(compiler->header, node->name_len);
        bb_append(compiler->header, node->name, node->name_len);
    }

    value = ht_search_string_int(compiler->strings, node->name, node->name_len);

    bb_add_byte(compiler->buffer, NEWSTR8);
    bb_intbytes8(compiler->buffer, value->value.ival);
}

static void visit_List(const Compiler *const compiler, const Node *const node) {
    bb_add_byte(compiler->buffer, NEWLIST);
    int i;
    for (i = 0; i < node->children[0]->children_len; i++) {
        bb_add_byte(compiler->buffer, DUP);
        visit(compiler, node->children[0]->children[i]);
        bb_add_byte(compiler->buffer, SWAP);
        bb_add_byte(compiler->buffer, MCALL_8);
        bb_intbytes8(compiler->buffer, M_APPEND); // NOTE: this depends on the M_APPEND constant in method.h
        bb_add_byte(compiler->buffer, POP);
    }
}

static void visit_Map(const Compiler *const compiler, const Node *const node) {
    bb_add_byte(compiler->buffer, NEWMAP);
    int i;
    for (i = 0; i < node->children[0]->children_len; i++) {
        bb_add_byte(compiler->buffer, DUP);
        visit(compiler, node->children[0]->children[i]);
        bb_add_byte(compiler->buffer, SWAP);
        visit(compiler, node->children[1]->children[i]);
        bb_add_byte(compiler->buffer, SWAP);
        bb_add_byte(compiler->buffer, MCALL_8);
        bb_intbytes8(compiler->buffer, M___SET);  // NOTE: depends on value of M___SET in methods.h
        bb_add_byte(compiler->buffer, POP);
    }
}


static void visit(const Compiler *const compiler, const Node *const node) {
    switch(node->nodetype) {
    case N_EXPRSTMT:
        YASL_TRACE_LOG("%s\n", "Visit ExprStmt");
        visit_ExprStmt(compiler, node);
        break;
    case N_BLOCK:
        YASL_TRACE_LOG("%s\n", "Visit Block");
        visit_Block(compiler, node);
        break;
    case N_FNDECL:
        YASL_TRACE_LOG("%s\n", "Visit FunctionDecl");
        visit_FunctionDecl(compiler, node);
        break;
    case N_CALL:
        YASL_TRACE_LOG("%s\n", "Visit Call");
        visit_Call(compiler, node);
        break;
    case N_RET:
        YASL_TRACE_LOG("%s\n", "Visit Return");
        visit_Return(compiler, node);
        break;
    case N_SET:
        YASL_TRACE_LOG("%s\n", "Visit Set");
        visit_Set(compiler, node);
        break;
    case N_GET:
        YASL_TRACE_LOG("%s\n", "Visit Get");
        visit_Get(compiler, node);
        break;
    case N_WHILE:
        YASL_TRACE_LOG("%s\n", "Visit While");
        visit_While(compiler, node);
        break;
    case N_BREAK:
        YASL_TRACE_LOG("%s\n", "Visit Break");
        visit_Break(compiler, node);
        break;
    case N_CONT:
        YASL_TRACE_LOG("%s\n", "Visit Continue");
        visit_Continue(compiler, node);
        break;
    case N_IF:
        YASL_TRACE_LOG("%s\n", "Visit If");
        visit_If(compiler, node);
        break;
    case N_PRINT:
        YASL_TRACE_LOG("%s\n", "Visit Print");
        visit_Print(compiler, node);
        break;
    case N_LET:
        YASL_TRACE_LOG("%s\n", "Visit Let");
        visit_Let(compiler, node);
        break;
    case N_TRIOP:
        YASL_TRACE_LOG("%s\n", "Visit TriOp");
        visit_TriOp(compiler, node);
        break;
    case N_BINOP:
        YASL_TRACE_LOG("%s\n", "Visit BinOp");
        visit_BinOp(compiler, node);
        break;
    case N_UNOP:
        YASL_TRACE_LOG("%s\n", "Visit UnOp");
        visit_UnOp(compiler, node);
        break;
    case N_ASSIGN:
        YASL_TRACE_LOG("%s\n", "Visit Assign");
        visit_Assign(compiler, node);
        break;
    case N_VAR:
        YASL_TRACE_LOG("%s\n", "Visit Var");
        visit_Var(compiler, node);
        break;
    case N_UNDEF:
        YASL_TRACE_LOG("%s\n", "Visit Undef");
        visit_Undef(compiler, node);
        break;
    case N_FLOAT64:
        YASL_TRACE_LOG("%s\n", "Visit Float");
        visit_Float(compiler, node);
        break;
    case N_INT64:
        YASL_TRACE_LOG("%s\n", "Visit Integer");
        visit_Integer(compiler, node);
        break;
    case N_BOOL:
        YASL_TRACE_LOG("%s\n", "Visit Boolean");
        visit_Boolean(compiler, node);
        break;
    case N_STR:
        YASL_TRACE_LOG("%s\n", "Visit String");
        visit_String(compiler, node);
        break;
    case N_LIST:
        YASL_TRACE_LOG("%s\n", "Visit List");
        visit_List(compiler, node);
        break;
    case N_MAP:
        YASL_TRACE_LOG("%s\n", "Visit Map");
        visit_Map(compiler, node);
        break;
    default:
        printf("%d\n", node->nodetype);
        puts("unknown node type");
        exit(EXIT_FAILURE);
    }
}