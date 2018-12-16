#include <debug.h>
#include "ast.h"

struct Node *node_clone(const struct Node *const node) {
    if (node == NULL) return NULL;
    struct Node *clone = malloc(sizeof(struct Node) + node->children_len*sizeof(struct Node*));
    clone->nodetype = node->nodetype;
    clone->type = node->type;
    clone->children_len = node->children_len;
    // clone->children = malloc(clone->children_len * sizeof(struct Node));
    for (size_t i = 0; i < clone->children_len; i++) {
        clone->children[i] = node_clone(node->children[i]);
    }
    clone->name_len = node->name_len;
    clone->name = malloc(node->name_len);
    memcpy(clone->name, node->name, clone->name_len);
    clone->line = node->line;
    return clone;
}


static struct Node *new_Node_0(AST nodetype, Token type, char *name /* OWN */, size_t name_len, size_t line) {
    struct Node *node = malloc(sizeof(struct Node));
    node->nodetype = nodetype;
    node->type = type;
    // node->children = NULL;
    node->children_len = 0;
    node->name_len = name_len;
    node->name = name;
    node->line = line;
    return node;
}

static struct Node *new_Node_1(AST nodetype, Token type, struct Node *child /* OWN */, char *name /* OWN */, int64_t name_len, size_t line) {
    struct Node *node = malloc(sizeof(struct Node) + sizeof(struct Node*));
    node->nodetype = nodetype;
    node->type = type;
    // node->children = malloc(1 * sizeof(struct Node*));
    node->children[0] = child;
    node->children_len = 1;
    node->name_len = name_len;
    node->name = name;
    node->line = line;
    return node;
}

static struct Node *new_Node_2(AST nodetype, Token type, struct Node *child1 /* OWN */, struct Node *child2 /* OWN */, char *name /* OWN */, int64_t name_len, size_t line) {
    struct Node *node = malloc(sizeof(struct Node) + 2* sizeof(struct Node*));
    node->nodetype = nodetype;
    node->type = type;
    // node->children = malloc(2 * sizeof(struct Node*));
    node->children[0] = child1;
    node->children[1] = child2;
    node->children_len = 2;
    node->name_len = name_len;
    node->name = name;
    node->line = line;
    return node;
}

static struct Node *new_Node_3(AST nodetype, Token type, struct Node *child1 /* OWN */, struct Node *child2 /* OWN */, struct Node *child3 /* OWN */, char *name /* OWN */, int64_t name_len, size_t line) {
    struct Node *node = malloc(sizeof(struct Node) + 3* sizeof(struct Node*));
    node->nodetype = nodetype;
    node->type = type;
    // node->children = malloc(3 * sizeof(struct Node*));
    node->children[0] = child1;
    node->children[1] = child2;
    node->children[2] = child3;
    node->children_len = 3;
    node->name_len = name_len;
    node->name = name;
    node->line = line;
    return node;
}


struct Node *new_ExprStmt(struct Node *child, size_t line) {
    return new_Node_1(N_EXPRSTMT, T_UNKNOWN, child, NULL, 0, line);
}

struct Node *new_Block(struct Node *body, size_t line) {
    return new_Node_1(N_BLOCK, T_UNKNOWN, body, NULL, 0, line);
}

struct Node *new_Body(size_t line) {
    return new_Node_0(N_BODY, T_UNKNOWN, NULL, 0, line);
}

void body_append(struct Node **node, struct Node *const child) {
    YASL_TRACE_LOG("%s\n", "appending to block");
    *node = realloc(*node, sizeof(struct Node) + (++(*node)->children_len)*sizeof(struct Node*));
    (*node)->children[(*node)->children_len-1] = child;
}


struct Node *new_FnDecl(struct Node *params, struct Node *body, char *name, int64_t name_len, size_t line) {
    return new_Node_2(N_FNDECL, T_UNKNOWN, params, body, name, name_len, line);
}

struct Node *new_Return(struct Node *expr, size_t line) {
    return new_Node_1(N_RET, T_UNKNOWN, expr, NULL, 0, line);
}

struct Node *new_Call(struct Node *params, struct Node *object, size_t line) {
    return new_Node_2(N_CALL, T_UNKNOWN, params, object, NULL, 0, line);
}

struct Node *new_Set(struct Node *collection, struct Node *key, struct Node *value, size_t line) {
    return new_Node_3(N_SET, T_UNKNOWN, collection, key, value, NULL, 0, line);
}

struct Node *new_Get(struct Node *collection, struct Node *value, size_t line) {
    return new_Node_2(N_GET, T_UNKNOWN, collection, value, NULL, 0, line);
}

struct Node *ListComp_get_expr(const struct Node *const node) {
    return node->children[0];
}

struct Node *ListComp_get_var(const struct Node *const node) {
    return node->children[1];
}

struct Node *ListComp_get_collection(const struct Node *const node) {
    return node->children[2];
}

struct Node *new_ListComp(struct Node *expr, struct Node *iter, struct Node *cond, size_t line) {
    return new_Node_3(N_LISTCOMP, T_UNKNOWN, expr, iter, cond, NULL, 0, line);
}

struct Node *TableComp_get_key_value(const struct Node *const node) {
    return node->children[0];
}

struct Node *new_TableComp(struct Node *expr, struct Node *iter, struct Node *cond, size_t line) {
    return new_Node_3(N_TABLECOMP, T_UNKNOWN, expr, iter, cond, NULL, 0, line);
}

struct Node *new_LetIter(struct Node *var, struct Node *collection, size_t line) {
    return new_Node_2(N_LETITER, T_UNKNOWN, var, collection, NULL, 0, line);
}

struct Node *new_Iter(struct Node *var, struct Node *collection, size_t line) {
    return new_Node_2(N_ITER, T_UNKNOWN, var, collection, NULL, 0, line);
}

struct Node *ForIter_get_var(const struct Node *const node) {
    return node->children[0];
}

struct Node *ForIter_get_collection(const struct Node *const node) {
    return node->children[1];
}

struct Node *ForIter_get_body(const struct Node *const node) {
    return node->children[1];
}

struct Node *new_ForIter(struct Node *iter, struct Node *body, size_t line) {
    return new_Node_2(N_FORITER, T_UNKNOWN, iter, body, NULL, 0, line);
}

struct Node *While_get_cond(const struct Node *const node) {
    return node->children[0];
}

struct Node *While_get_body(const struct Node *const node) {
    return node->children[1];
}

struct Node *new_While(struct Node *cond, struct Node *body, struct Node *post, size_t line) {
    return new_Node_3(N_WHILE, T_UNKNOWN, cond, body, post, NULL, 0, line);
}

struct Node *new_Break(size_t line) {
    return new_Node_0(N_BREAK, T_UNKNOWN, NULL, 0, line);
}

struct Node *new_Continue(size_t line) {
    return new_Node_0(N_CONT, T_UNKNOWN, NULL, 0, line);
}

struct Node *new_If(struct Node *cond, struct Node *then_node, struct Node *else_node, size_t line) {
    return new_Node_3(N_IF, T_UNKNOWN, cond, then_node, else_node, NULL, 0, line);
}

struct Node *Print_get_expr(const struct Node *const node) {
    return node->children[0];
}

struct Node *new_Print(struct Node *expr, size_t line) {
    return new_Node_1(N_PRINT, T_UNKNOWN, expr, NULL, 0, line);
}

struct Node *Let_get_expr(const struct Node *const node) {
    return node->children[0];
}

struct Node *new_Let(char *name, int64_t name_len, struct Node *expr, size_t line) {
    return new_Node_1(N_LET, T_UNKNOWN, expr, name, name_len, line);
}

struct Node *Const_get_expr(struct Node *node) {
    return node->children[0];
}

struct Node *new_Const(char *name, int64_t name_len, struct Node *expr, size_t line) {
    return new_Node_1(N_CONST, T_UNKNOWN, expr, name, name_len, line);
}

struct Node *new_TriOp(Token op, struct Node *left, struct Node *middle, struct Node *right, size_t line) {
    return new_Node_3(N_TRIOP, op, left, middle, right, NULL, 0, line);
}

struct Node *new_BinOp(Token op, struct Node *left, struct Node *right, size_t line) {
    return new_Node_2(N_BINOP, op, left, right, NULL, 0, line);
}

struct Node *new_UnOp(Token op, struct Node *child, size_t line) {
    return new_Node_1(N_UNOP, op, child, NULL, 0, line);
}

struct Node *new_Assign(char *name, int64_t name_len, struct Node *child, size_t line) {
    return new_Node_1(N_ASSIGN, T_UNKNOWN, child, name, name_len, line);
}

struct Node *new_Var(char *name, int64_t name_len, size_t line) {
    return new_Node_0(N_VAR, T_UNKNOWN, name, name_len, line);
}

struct Node *new_Undef(size_t line) {
    return new_Node_0(N_UNDEF, T_UNKNOWN, NULL, 0, line);
}

struct Node *new_Float(char *value, size_t len, size_t line) {
    return new_Node_0(N_FLOAT64, T_UNKNOWN, value, len, line);
}

struct Node *new_Integer(char *value, size_t len, size_t line) {
    return new_Node_0(N_INT64, T_UNKNOWN, value, len, line);
}

struct Node *new_Boolean(char *value, size_t len, size_t line) {
    return new_Node_0(N_BOOL, T_UNKNOWN, value, len, line);
}

struct Node *new_String(char *value, size_t len, size_t line) {
    return new_Node_0(N_STR, T_UNKNOWN, value, len, line);
}

struct Node *List_get_values(const struct Node *const node) {
    return node->children[0];
}

struct Node *new_List(struct Node *values, size_t line) {
    return new_Node_1(N_LIST, T_UNKNOWN, values, NULL, 0, line);
}

struct Node *Table_get_values(const struct Node *const node) {
    return node->children[0];
}

struct Node *new_Table(struct Node *keys, size_t line) {
    return new_Node_1(N_TABLE, T_UNKNOWN, keys, NULL, 0, line);
}

void node_del(struct Node *node) {
    if (!node) return;
    while(node->children_len-- > 0)
        if (node->children[node->children_len] != NULL)
            node_del(node->children[node->children_len]);
    free(node->name);
    // free(node->children);
    free(node);

}
