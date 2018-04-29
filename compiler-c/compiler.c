#include "compiler.h"



Compiler *compiler_new(Parser* parser) {
    Compiler *compiler = malloc(sizeof(Compiler));
    /*compiler->globals = env_new();
    compiler->locals = env_new();
    compiler->header = NULL;
    compiler->header_len = 0;
    compiler->code = NULL;
    compiler->code_len = 0; */
    compiler->parser = parser;
    compiler->buffer = bb_new(16);
    compiler->header = bb_new(16);
    compiler->header->count = 16;
    compiler->code   = bb_new(16);
    return compiler;
};

void compiler_del(Compiler *compiler) {
    /*env_del(compiler->globals);
    env_del(compiler->locals);
    free(compiler->header);
    free(compiler->code); */
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
    while (!peof(compiler->parser)) {
            gettok(compiler->parser->lex);
            if (peof(compiler->parser)) break;
            node = parse(compiler->parser);
            visit(compiler, node);
            bb_append(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
            compiler->buffer->count = 0;
            node_del(node);
    }
    memcpy(compiler->header->bytes, &compiler->header->count, sizeof(int64_t));
    int i = 0;
    for (i = 0; i < compiler->header->count; i++) {
        printf("%02x\n", compiler->header->bytes[i]);// && 0xFF);
    }
    puts("entry point");
    for (i = 0; i < compiler->code->count; i++) {
        printf("%02x\n", compiler->code->bytes[i]);// & 0xFF);
    }
    printf("%02x\n", HALT);
    FILE *fp = fopen("../source.yb", "wb");
    if (!fp) exit(EXIT_FAILURE);
    fwrite(compiler->header->bytes, 1, compiler->header->count, fp);
    fwrite(compiler->code->bytes, 1, compiler->code->count, fp);
    fputc(HALT, fp);
    fclose(fp);
    /*   FILE *fp;
   char str[] = "This is tutorialspoint.com";

   fp = fopen( "file.txt" , "w" );
   fwrite(str , 1 , sizeof(str) , fp );

   fclose(fp);

   return(0); */
}

void visit_Print(Compiler* compiler, Node *node) {
    visit(compiler, node->children[0]);
    printf("print\n");
    char print_bytes[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    bb_add_byte(compiler->buffer, BCALL_8);
    bb_append(compiler->buffer, print_bytes, 8);
    //free(print_bytes);
}

void visit_String(Compiler* compiler, Node *node) {
    puts("visit string");
    //printf("%s", node->name);
    // [NEWSTR8] + self.strs[node.value]["length"] + self.strs[node.value]["addr"]
    bb_add_byte(compiler->buffer, NEWSTR8);
    bb_append(compiler->buffer, (char*)&node->name_len, sizeof(int64_t));
    bb_append(compiler->buffer, (char*)&compiler->header->count, sizeof(int64_t));
    bb_append(compiler->header, node->name, node->name_len);
    //bb_add_byte()
    //printf("%s", compiler->header->bytes + 16);
    //char string_bytes[] = {};
    //bb_append(compiler->buffer, )
    printf("string\n");
}

void visit(Compiler* compiler, Node* node) {
    // puts("visit");
    switch(node->nodetype) {
    case NODE_PRINT:
        visit_Print(compiler, node);
        break;
    case NODE_STR:
        visit_String(compiler, node);
        break;
    default:
        exit(1);
    }
}