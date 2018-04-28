#include "ast.h"


/*
 * Node {
 *   AST nodetype;
 *   Token type;
 *   Node *children;
 *   char* name;
 *   int len;
 * }
 */

Node *new_TriOp(Token op, left, middle, right) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = TRIOP;
    node->type = op;
    node->children = malloc(sizeof(Node*)*3);
    node->children[0] = left;
    node->children[1] = middle;
    node->children[2] = right;
    node->name = NULL;
    return node;
};

Node *new_BinOp(Token op, left, right) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = TRIOP;
    node->type = op;
    node->children = malloc(sizeof(Node*)*2);
    node->children[0] = left;
    node->children[1] = right;
    node->name = NULL;
    return node;
};

Node *new_UnOp(Token op, child) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = TRIOP;
    node->type = op;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name = NULL;
    return node;
};

Node *new_Integer(char *value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = CONST;
    node->type = TOK_INT64;
    Node *children = NULL;
    memcpy(node->name, value, node->len = len);
    return node;
}