#include <interpreter/YASL_Object/YASL_Object.h>
#include <interpreter/YASL_string/YASL_string.h>
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

    // TODO: deal with memory leaks here.
    compiler->builtins = new_hash();
    ht_insert_string_int(compiler->builtins, "open", 4, F_OPEN);
    ht_insert_string_int(compiler->builtins, "popen", 5, F_POPEN);
    ht_insert_string_int(compiler->builtins, "input", 5, F_INPUT);

    compiler->methods = new_hash();
    ht_insert_string_int(compiler->methods, "tofloat64", 9, M_TOFLOAT64);
    ht_insert_string_int(compiler->methods, "toint64", 7, M_TOINT64);
    ht_insert_string_int(compiler->methods, "tobool", 6, M_TOBOOL);
    ht_insert_string_int(compiler->methods, "tostr", 5, M_TOSTR);

    ht_insert_string_int(compiler->methods, "upcase", 6, M_UPCASE);
    ht_insert_string_int(compiler->methods, "downcase", 8, M_DOWNCASE);
    ht_insert_string_int(compiler->methods, "isalnum", 7, M_ISALNUM);
    ht_insert_string_int(compiler->methods, "isal", 4, M_ISAL);
    ht_insert_string_int(compiler->methods, "isnum", 5, M_ISNUM);
    ht_insert_string_int(compiler->methods, "isspace", 7, M_ISSPACE);
    ht_insert_string_int(compiler->methods, "startswith", 10, M_STARTSWITH);
    ht_insert_string_int(compiler->methods, "endswith", 8, M_ENDSWITH);
    ht_insert_string_int(compiler->methods, "search", 6, M_SEARCH);
    ht_insert_string_int(compiler->methods, "split", 5, M_SPLIT);

    ht_insert_string_int(compiler->methods, "append", 6, M_APPEND);

    ht_insert_string_int(compiler->methods, "keys", 4, M_KEYS);
    ht_insert_string_int(compiler->methods, "values", 6, M_VALUES);

    ht_insert_string_int(compiler->methods, "close", 5, M_CLOSE);
    ht_insert_string_int(compiler->methods, "pclose", 6, M_PCLOSE);
    ht_insert_string_int(compiler->methods, "read", 4, M_READ);
    ht_insert_string_int(compiler->methods, "write", 5, M_WRITE);
    ht_insert_string_int(compiler->methods, "readline", 8, M_READLINE);

    ht_insert_string_int(compiler->methods, "__set", 5, M___SET);
    ht_insert_string_int(compiler->methods, "__get", 5, M___GET);

    compiler->strings = new_hash();
    compiler->parser = parser;
    compiler->buffer = bb_new(16);
    compiler->header = bb_new(16);
    compiler->header->count = 16;
    compiler->checkpoints_size = 4;
    compiler->checkpoints = malloc(sizeof(int64_t)*compiler->checkpoints_size);
    compiler->checkpoints_count = 0;
    compiler->code   = bb_new(16);
    return compiler;
};

void compiler_del(Compiler *compiler) {

    int i;
    for (i = 0; i < compiler->strings->size; i++) {
        Item_t* item = compiler->strings->items[i];
        if (item != NULL) {
            del_string8(item->key->value.sval);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    free(compiler->strings->items);
    free(compiler->strings);

    for (i = 0; i < compiler->builtins->size; i++) {
        Item_t* item = compiler->builtins->items[i];
        if (item != NULL) {
            del_string8(item->key->value.sval);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    free(compiler->builtins->items);
    free(compiler->builtins);

    for (i = 0; i < compiler->methods->size; i++) {
        Item_t* item = compiler->methods->items[i];
        if (item != NULL) {
            del_string8(item->key->value.sval);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    free(compiler->methods->items);
    free(compiler->methods);

    env_del(compiler->globals);
    env_del(compiler->locals);
    
    parser_del(compiler->parser);

    bb_del(compiler->buffer);
    bb_del(compiler->header);
    bb_del(compiler->code);

    free(compiler->checkpoints);

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
            eattok(compiler->parser, T_SEMI);
            YASL_DEBUG_LOG("eaten semi. type: %s, ", YASL_TOKEN_NAMES[compiler->parser->lex->type]);
            YASL_DEBUG_LOG("value: %s\n", compiler->parser->lex->value);
            YASL_DEBUG_LOG("%s\n", "about to visit.");
            visit(compiler, node);
            YASL_DEBUG_LOG("%s\n", "visited.");
            bb_append(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
            compiler->buffer->count = 0;
            node_del(node);
    }
    bb_rewrite_intbytes8(compiler->header, 0, compiler->header->count);
    int i = 0;
    for (i = 0; i < compiler->header->count; i++) {
        YASL_DEBUG_LOG("%02x\n", compiler->header->bytes[i]);
    }
    YASL_DEBUG_LOG("%s\n", "entry point");
    for (i = 0; i < compiler->code->count; i++) {
        YASL_DEBUG_LOG("%02x\n", compiler->code->bytes[i]);
    }
    YASL_DEBUG_LOG("%02x\n", HALT);
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

void visit_FunctionDecl(Compiler *compiler, Node *node) {
    /*if (compiler->current_function != NULL) {
        puts("Illegal function declaration outside global scope.");
        exit(EXIT_FAILURE);
    }
    compiler->current_function = realloc(compiler->current_function, node->name_len);
    memcpy(compiler->current_function, node->name, node->name_len);
    */

    puts("User defined functions are currently not implemented.");
    exit(EXIT_FAILURE);
}

void visit_Call(Compiler *compiler, Node *node) {
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
    YASL_DEBUG_LOG("visiting call %s\n", node->name);
    visit_Block(compiler, node->children[0]);

    if (ht_search_string_int(compiler->builtins, node->name, node->name_len)) {
        bb_add_byte(compiler->buffer, BCALL_8);
        bb_intbytes8(compiler->buffer, ht_search_string_int(compiler->builtins, node->name, node->name_len)->value.ival);
    } else {
        // TODO: implement non-builtins.
        puts("Not a builtin function.");
        exit(EXIT_FAILURE);
    }
}

/*
    def visit_MethodCall(self, node):
        result = []
        for expr in node.params:
            result = result + self.visit(expr)
        if node.value in METHODS:
            return result + self.visit(node.left) + [MCALL_8] + intbytes_8(METHODS[node.value])
        assert False
 */


void visit_Method(Compiler *compiler, Node *node) {
    YASL_DEBUG_LOG("visiting method %s\n", node->name);
    visit_Block(compiler, node->children[1]);
    visit(compiler, node->children[0]);

    if (ht_search_string_int(compiler->methods, node->name, node->name_len)) {
        bb_add_byte(compiler->buffer, MCALL_8);
        bb_intbytes8(compiler->buffer, ht_search_string_int(compiler->methods, node->name, node->name_len)->value.ival);
    } else {
        // TODO: implement non-builtins.
        printf("No builtin method `%s`\n", node->name);
        exit(EXIT_FAILURE);
    }
}

void visit_Index(Compiler *compiler, Node *node) {
    visit(compiler, node->children[1]);
    visit(compiler, node->children[0]);
    bb_add_byte(compiler->buffer, MCALL_8);
    bb_intbytes8(compiler->buffer, M___GET);
}

void visit_Block(Compiler *compiler, Node *node) {
    YASL_DEBUG_LOG("the block has %d children.\n", node->children_len);
    int i;
    for (i = 0; i < node->children_len; i++) {
        visit(compiler, node->children[i]);
    }
}

void visit_While(Compiler *compiler, Node *node) {
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
}

void visit_If(Compiler *compiler, Node *node) {
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

void visit_Print(Compiler* compiler, Node *node) {
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
void visit_Let(Compiler *compiler, Node *node) {
    if (!env_contains(compiler->globals, node->name, node->name_len)) {
        env_decl_var(compiler->globals, node->name, node->name_len);
    }
    if (node->children != NULL) visit(compiler, node->children[0]);
    else bb_add_byte(compiler->buffer, NCONST);
    // TODO: handle locals
    bb_add_byte(compiler->buffer, GSTORE_1);
    bb_add_byte(compiler->buffer, env_get(compiler->globals, node->name, node->name_len));  // TODO: deal with size
}

void visit_TriOp(Compiler *compiler, Node *node) {
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

void visit_BinOp(Compiler *compiler, Node *node) {
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

void visit_UnOp(Compiler *compiler, Node *node) {
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

void visit_Assign(Compiler *compiler, Node *node) {
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
    bb_add_byte(compiler->buffer, env_get(compiler->globals, node->name, node->name_len));  // TODO: handle size
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

void visit_String(Compiler* compiler, Node *node) {
    YASL_Object *value = ht_search_string_int(compiler->strings, node->name, node->name_len);
    if (value == NULL) {
        YASL_DEBUG_LOG("%s\n", "caching string");
        ht_insert_string_int(compiler->strings, node->name, node->name_len, compiler->header->count);
        bb_append(compiler->header, node->name, node->name_len);
    }

    value = ht_search_string_int(compiler->strings, node->name, node->name_len);

    bb_add_byte(compiler->buffer, NEWSTR8);
    bb_intbytes8(compiler->buffer, node->name_len);
    bb_intbytes8(compiler->buffer, value->value.ival);
}

void visit_List(Compiler *compiler, Node *node) {
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

void visit_Map(Compiler *compiler, Node *node) {
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



void visit(Compiler* compiler, Node* node) {
    switch(node->nodetype) {
    case N_EXPRSTMT:
        YASL_DEBUG_LOG("%s\n", "Visit ExprStmt");
        visit_ExprStmt(compiler, node);
        break;
    case N_BLOCK:
        YASL_DEBUG_LOG("%s\n", "Visit Block");
        visit_Block(compiler, node);
        break;
    case N_FNDECL:
        YASL_DEBUG_LOG("%s\n", "Visit FunctionDecl");
        visit_FunctionDecl(compiler, node);
        break;
    case N_CALL:
        YASL_DEBUG_LOG("%s\n", "Visit Call");
        visit_Call(compiler, node);
        break;
    case N_METHOD:
        YASL_DEBUG_LOG("%s\n", "Visit Method Call");
        visit_Method(compiler, node);
        break;
    case N_INDEX:
        YASL_DEBUG_LOG("%s\n", "Visit Index");
        visit_Index(compiler, node);
        break;
    case N_WHILE:
        YASL_DEBUG_LOG("%s\n", "Visit While");
        visit_While(compiler, node);
        break;
    case N_BREAK:
        YASL_DEBUG_LOG("%s\n", "Visit Break");
        visit_Break(compiler, node);
        break;
    case N_CONT:
        YASL_DEBUG_LOG("%s\n", "Visit Continue");
        visit_Continue(compiler, node);
        break;
    case N_IF:
        YASL_DEBUG_LOG("%s\n", "Visit If");
        visit_If(compiler, node);
        break;
    case N_PRINT:
        YASL_DEBUG_LOG("%s\n", "Visit Print");
        visit_Print(compiler, node);
        break;
    case N_LET:
        YASL_DEBUG_LOG("%s\n", "Visit Let");
        visit_Let(compiler, node);
        break;
    case N_TRIOP:
        YASL_DEBUG_LOG("%s\n", "Visit TriOp");
        visit_TriOp(compiler, node);
        break;
    case N_BINOP:
        YASL_DEBUG_LOG("%s\n", "Visit BinOp");
        visit_BinOp(compiler, node);
        break;
    case N_UNOP:
        YASL_DEBUG_LOG("%s\n", "Visit UnOp");
        visit_UnOp(compiler, node);
        break;
    case N_ASSIGN:
        YASL_DEBUG_LOG("%s\n", "Visit Assign");
        visit_Assign(compiler, node);
        break;
    case N_VAR:
        YASL_DEBUG_LOG("%s\n", "Visit Var");
        visit_Var(compiler, node);
        break;
    case N_UNDEF:
        YASL_DEBUG_LOG("%s\n", "Visit Undef");
        visit_Undef(compiler, node);
        break;
    case N_FLOAT64:
        YASL_DEBUG_LOG("%s\n", "Visit Float");
        visit_Float(compiler, node);
        break;
    case N_INT64:
        YASL_DEBUG_LOG("%s\n", "Visit Integer");
        visit_Integer(compiler, node);
        break;
    case N_BOOL:
        YASL_DEBUG_LOG("%s\n", "Visit Boolean");
        visit_Boolean(compiler, node);
        break;
    case N_STR:
        YASL_DEBUG_LOG("%s\n", "Visit String");
        visit_String(compiler, node);
        break;
    case N_LIST:
        YASL_DEBUG_LOG("%s\n", "Visit List");
        visit_List(compiler, node);
        break;
    case N_MAP:
        YASL_DEBUG_LOG("%s\n", "Visit Map");
        visit_Map(compiler, node);
        break;
    default:
        printf("%d\n", node->nodetype);
        puts("unknown node type");
        exit(EXIT_FAILURE);
    }
}