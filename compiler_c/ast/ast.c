#include "ast.h"



int main(void) {
    TriOpNode *node = ast_new_TriOpNode(0, NULL, NULL, NULL);
    printf("%d\n", node->nodetype);
}