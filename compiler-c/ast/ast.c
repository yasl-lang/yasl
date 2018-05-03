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
}

Node *new_Let(char *name, int64_t name_len, Node *child) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_LET;
    node->type = TOK_LET;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name = malloc(sizeof(char)*(node->name_len = name_len));
    memcpy(node->name, name, node->name_len);
    return node;
}

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
}

Node *new_Var(char *name, int64_t name_len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_VAR;
    node->type = TOK_ID;
    node->children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = name_len));
    memcpy(node->name, name, node->name_len);
    return node;
}

Node *new_Undef(void) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_UNDEF;
    node->type = TOK_UNDEF;
    node->children = NULL;
    node->name = NULL;
    return node;
}

Node *new_Float(char *value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_FLOAT64;
    node->type = TOK_FLOAT64;
    node->children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = len));
    memcpy(node->name, value, node->name_len);
    return node;
}

Node *new_Integer(char *value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_INT64;
    node->type = TOK_INT64;
    node->children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = len));
    memcpy(node->name, value, node->name_len);
    return node;
}

Node *new_Boolean(char *value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_BOOL;
    node->type = TOK_BOOL;
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

void del_Let(Node *node) {
    if (node->children[0] != NULL) node_del(node->children[0]);
    free(node->children);
    free(node->name);
    free(node);
}

void del_BinOp(Node *node) {
    node_del(node->children[0]);
    node_del(node->children[1]);
    free(node->children);
    free(node);
}

void del_UnOp(Node *node) {
    node_del(node->children[0]);
    free(node->children);
    free(node);
}

void del_Var(Node *node) {
    free(node->name);
    free(node);
}

void del_Undef(Node *node) {
    free(node);
}
// void del_TriOp(Node *node);
void del_Float(Node *node) {
    free(node->name);
    free(node);
}

void del_Integer(Node *node) {
    free(node->name);
    free(node);
}

void del_Boolean(Node *node) {
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
    case NODE_LET:
        del_Let(node);
        break;
    case NODE_BINOP:
        del_BinOp(node);
        break;
    case NODE_UNOP:
        del_UnOp(node);
        break;
    case NODE_VAR:
        del_Var(node);
        break;
    case NODE_UNDEF:
        del_Undef(node);
        break;
    case NODE_FLOAT64:
        del_Float(node);
        break;
    case NODE_INT64:
        del_Integer(node);
        break;
    case NODE_BOOL:
        del_Boolean(node);
        break;
    case NODE_STR:
        del_String(node);
        break;
    default:
        puts("Error in node_del: Unknown type");
        exit(1);
    }
}
