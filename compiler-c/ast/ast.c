#include "ast.h"


/*
 * Node {
 *   AST nodetype;
 *   Token type;
 *   Node *children;
 *   char* name;
 *   int len;
 *   int line;
 * }
 */

Node *new_Print(Node *child) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_PRINT;
    node->type = TOK_PRINT;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name = NULL;
    return node;
};

Node *new_TriOp(Token op, Node *left, Node *middle, Node *right) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_TRIOP;
    node->type = op;
    node->children = malloc(sizeof(Node*)*3);
    node->children[0] = left;
    node->children[1] = middle;
    node->children[2] = right;
    node->name = NULL;
    return node;
};

Node *new_BinOp(Token op, Node *left, Node *right) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_BINOP;
    node->type = op;
    node->children = malloc(sizeof(Node*)*2);
    node->children[0] = left;
    node->children[1] = right;
    node->name = NULL;
    return node;
};

Node *new_UnOp(Token op, Node *child) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_UNOP;
    node->type = op;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name = NULL;
    return node;
};

Node *new_Integer(char *value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_INT64;
    node->type = TOK_INT64;
    node->children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = len));
    memcpy(node->name, value, node->name_len);
    return node;
}

Node *new_String(char* value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_STR;
    node->type = TOK_STR;
    Node *children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = len));
    memcpy(node->name, value, node->name_len);
    return node;
}

void del_Print(Node *node) {
    node_del(node->children[0]);
    free(node->children);
    free(node);
}

// void del_TriOp(Node *node);
void del_Integer(Node *node) {
    free(node->name);
    free(node);
}

void del_String(Node *node) {
    free(node->name);
    free(node);
}
void node_del(Node *node) {
    if (node == NULL) return;
    switch (node->nodetype) {
    case NODE_PRINT:
        del_Print(node);
        break;
    case NODE_INT64:
        del_Integer(node);
        break;
    case NODE_STR:
        del_String(node);
        break;
    default:
        puts("Error in node_del: Unknown type");
        exit(1);
    }
}
