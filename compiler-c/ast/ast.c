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

Node *new_Node_0(AST nodetype, Token type, char *name, int64_t name_len, int line) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = nodetype;
    node->type = type;
    node->children = NULL;
    node->name_len = name_len;
    if (name == NULL) {
        node->name = NULL;
    } else {
        node->name = malloc(node->name_len);
        memcpy(node->name, name, node->name_len);
    }
    node->line = line;
    return node;
}

Node *new_Node_1(AST nodetype, Token type, Node *child, char *name, int64_t name_len, int line) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = nodetype;
    node->type = type;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name_len = name_len;
    if (name == NULL) {
        node->name = NULL;
    } else {
        node->name = malloc(node->name_len);
        memcpy(node->name, name, node->name_len);
    }
    node->line = line;
    return node;
}

Node *new_Node_2(AST nodetype, Token type, Node *child1, Node *child2, char *name, int64_t name_len, int line) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = nodetype;
    node->type = type;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child1;
    node->children[1] = child2;
    node->name_len = name_len;
    if (name == NULL) {
        node->name = NULL;
    } else {
        node->name = malloc(node->name_len);
        memcpy(node->name, name, node->name_len);
    }
    node->line = line;
    return node;
}

Node *new_Node_3(AST nodetype, Token type, Node *child1, Node *child2, Node *child3, char *name, int64_t name_len, int line) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = nodetype;
    node->type = type;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child1;
    node->children[1] = child2;
    node->children[2] = child3;
    node->name_len = name_len;
    if (name == NULL) {
        node->name = NULL;
    } else {
        node->name = malloc(node->name_len);
        memcpy(node->name, name, node->name_len);
    }
    node->line = line;
    return node;
}


Node *new_ExprStmt(Node *child, int line) {
    return new_Node_1(NODE_EXPRSTMT, UNKNOWN, child, NULL, 0, line);
}

Node *new_Block(int line) {
    return new_Node_0(NODE_BLOCK, UNKNOWN, NULL, 0, line);
    /*Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_BLOCK;
    node->children = NULL;
    node->children_len = 0;
    node->name = NULL;
    return node; */
}

void block_append(Node *node, Node *child) {
    node->children = realloc(node->children, (++node->children_len)*sizeof(Node*));  //TODO: make better implementation
    node->children[node->children_len-1] = child;
}

Node *new_While(Node *cond, Node *body, int line) {
    return new_Node_2(NODE_WHILE, UNKNOWN, cond, body, NULL, 0, line);
}

/*
Node *new_While(Node *cond, Node *body) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_WHILE;
    node->children = malloc(sizeof(Node*)*2);
    node->children[0] = cond;
    node->children[1] = body;
    node->name = NULL;
    return node;
} */

Node *new_Break(int line) {
    return new_Node_0(NODE_BREAK, UNKNOWN, NULL, 0, line);
}

/*
Node *new_Break(void) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_BREAK;
    return node;
} */

Node *new_Continue(int line) {
    return new_Node_0(NODE_CONT, UNKNOWN, NULL, 0, line);
}

/*
Node *new_Continue(void) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_CONT;
    return node;
}
*/

Node *new_If(Node *cond, Node *then_node, Node *else_node, int line) {
    return new_Node_3(NODE_IF, UNKNOWN, cond, then_node, else_node, NULL, 0, line);
}

/*
Node *new_If(Node *cond, Node *then_node, Node *else_node) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_IF;
    node->children = malloc(sizeof(Node*)*3);
    node->children[0] = cond;
    node->children[1] = then_node;
    node->children[2] = else_node;
    node->name = NULL;
    return node;
} */

Node *new_Print(Node *child, int line) {
    return new_Node_1(NODE_PRINT, UNKNOWN, child, NULL, 0, line);
}

/*
Node *new_Print(Node *child) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_PRINT;
    node->type = TOK_PRINT;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name = NULL;
    return node;
}
 */

Node *new_Let(char *name, int64_t name_len, Node *child, int line) {
    return new_Node_1(NODE_LET, UNKNOWN, child, name, name_len, line);
}

/*
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
 */

Node *new_TriOp(Token op, Node *left, Node *middle, Node *right, int line) {
    return new_Node_3(NODE_TRIOP, op, left, middle, right, NULL, 0, line);
}

/*
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
*/

Node *new_BinOp(Token op, Node *left, Node *right, int line) {
    return new_Node_2(NODE_BINOP, op, left, right, NULL, 0, line);
}

/*
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
 */

Node *new_UnOp(Token op, Node *child, int line) {
    return new_Node_1(NODE_UNOP, op, child, NULL, 0, line);
}

/*
Node *new_UnOp(Token op, Node *child) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_UNOP;
    node->type = op;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name = NULL;
    return node;
}
*/

Node *new_Assign(char *name, int64_t name_len, Node *child, int line) {
    return new_Node_1(NODE_ASSIGN, UNKNOWN, child, name, name_len, line);
}

/*
Node *new_Assign(char *name, int64_t name_len, Node *child) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_ASSIGN;
    node->type = TOK_ID;
    node->children = malloc(sizeof(Node*));
    node->children[0] = child;
    node->name = malloc(sizeof(char)*(node->name_len = name_len));
    memcpy(node->name, name, node->name_len);
    return node;
}
 */

Node *new_Var(char *name, int64_t name_len, int line) {
    return new_Node_0(NODE_VAR, UNKNOWN, name, name_len, line);
}

/*
Node *new_Var(char *name, int64_t name_len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_VAR;
    node->type = TOK_ID;
    node->children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = name_len));
    memcpy(node->name, name, node->name_len);
    return node;
}
 */

Node *new_Undef(int line) {
    return new_Node_0(NODE_UNDEF, UNKNOWN, NULL, 0, line);
}

/*
Node *new_Undef(void) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_UNDEF;
    node->type = TOK_UNDEF;
    node->children = NULL;
    node->name = NULL;
    return node;
}
 */

Node *new_Float(char *value, int len, int line) {
    return new_Node_0(NODE_FLOAT64, UNKNOWN, value, len, line);
}

/*
Node *new_Float(char *value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_FLOAT64;
    node->type = TOK_FLOAT64;
    node->children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = len));
    memcpy(node->name, value, node->name_len);
    return node;
}
 */

Node *new_Integer(char *value, int len, int line) {
    return new_Node_0(NODE_INT64, UNKNOWN, value, len, line);
}

/*
Node *new_Integer(char *value, int len) {
    Node *node = malloc(sizeof(Node));
    memset(node, 0, sizeof(node));
    node->nodetype = NODE_INT64;
    node->type = TOK_INT64;
    node->children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = len));
    memcpy(node->name, value, node->name_len);
    return node;
}
 */

Node *new_Boolean(char *value, int len, int line) {
    return new_Node_0(NODE_BOOL, UNKNOWN, value, len, line);
}

/*
Node *new_Boolean(char *value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_BOOL;
    node->type = TOK_BOOL;
    node->children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = len));
    memcpy(node->name, value, node->name_len);
    return node;
}
 */

Node *new_String(char *value, int len, int line) {
    return new_Node_0(NODE_STR, UNKNOWN, value, len, line);
}

/*
Node *new_String(char* value, int len) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_STR;
    node->type = TOK_STR;
    Node *children = NULL;
    node->name = malloc(sizeof(char)*(node->name_len = len));
    memcpy(node->name, value, node->name_len);
    return node;
}
*/

Node *new_List(Node *values, int line) {
    return new_Node_1(NODE_LIST, UNKNOWN, values, NULL, 0, line);
}

/*
Node *new_List(Node *values) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_LIST;
    node->children = malloc(sizeof(Node*));
    node->children[0] = values;
    node->name = NULL;
    return node;
}
 */

Node *new_Map(Node *keys, Node *values, int line) {
    return new_Node_2(NODE_MAP, UNKNOWN, keys, values, NULL, 0, line);
}

/*
Node *new_Map(Node *keys, Node *values) {
    Node *node = malloc(sizeof(Node));
    node->nodetype = NODE_MAP;
    node->children = malloc(sizeof(Node*)*2);
    node->children[0] = keys;
    node->children[1] = values;
    node->name = NULL;
    return node;
}
 */

void del_ExprStmt(Node *node) {
    node_del(node->children[0]);
    free(node->children);
    free(node);
}

void del_Block(Node *node) {
    int i;
    for (i = 0; i < node->children_len; i++) {
        node_del(node->children[i]);
    }
    free(node->children);
    free(node);
}

void del_While(Node *node) {
    node_del(node->children[0]);
    node_del(node->children[1]);
    free(node->children);
    free(node);
}

void del_Break(Node *node) {
    free(node);
}

void del_Continue(Node *node) {
    free(node);
}

void del_If(Node *node) {
    node_del(node->children[0]);
    node_del(node->children[1]);
    if (node->children[2] != NULL)
        node_del(node->children[2]);
    free(node->children);
    free(node);
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


void del_TriOp(Node *node) {
    node_del(node->children[0]);
    node_del(node->children[1]);
    node_del(node->children[2]);
    free(node->children);
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

void del_Assign(Node *node) {
    node_del(node->children[0]);
    free(node->children);
    free(node->name);
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

void del_List(Node *node) {
    node_del(node->children[0]);
    free(node);
}

void del_Map(Node *node) {
    node_del(node->children[0]);
    node_del(node->children[1]);
    free(node);
}

void node_del(Node *node) {
/*
struct Node_s {
    AST nodetype;
    Token type;
    struct Node_s **children;
    int64_t children_len;
    char* name;
    int64_t name_len;
    int line;
};*/
    while(node->children_len-- > 0)
        if (node->children[node->children_len] != NULL)
            node_del(node->children[node->children_len]);
    free(node->name);
    free(node->children);
    free(node);
}

/*
void node_del(Node *node) {
    if (node == NULL) return;
    switch (node->nodetype) {
    case NODE_EXPRSTMT:
        del_ExprStmt(node);
        break;
    case NODE_BLOCK:
        del_Block(node);
        break;
    case NODE_WHILE:
        del_While(node);
        break;
    case NODE_BREAK:
        del_Break(node);
        break;
    case NODE_CONT:
        del_Continue(node);
        break;
    case NODE_IF:
        del_If(node);
        break;
    case NODE_PRINT:
        del_Print(node);
        break;
    case NODE_LET:
        del_Let(node);
        break;
    case NODE_TRIOP:
        del_TriOp(node);
        break;
    case NODE_BINOP:
        del_BinOp(node);
        break;
    case NODE_UNOP:
        del_UnOp(node);
        break;
    case NODE_ASSIGN:
        del_Assign(node);
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
    case NODE_LIST:
        del_List(node);
        break;
    case NODE_MAP:
        del_Map(node);
        break;
    default:
        puts("Error in node_del: Unknown type");
        exit(1);
    }
}
*/
