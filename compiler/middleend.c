#include "middleend.h"

void fold_ExprStmt(struct Node *const node) {
	fold(ExprStmt_get_expr(node));
}

void fold_Block(struct Node *const node) {
	fold(node->children[0]);
}

void fold_Body(struct Node *const node) {
	FOR_CHILDREN(i, child, node) {
		fold(child);
	}
}

void fold_Call(struct Node *const node) {
	fold(node->children[1]);
	fold_Body(Call_get_params(node));
}

void fold_MethodCall(struct Node *const node) {
	fold(node->children[1]);
	fold_Body(node->children[0]);
}

void fold_Set(struct Node *const node) {
	fold(Set_get_collection(node));
	fold(Set_get_key(node));
	fold(Set_get_value(node));
}

void fold_Get(struct Node *const node) {
	fold(Get_get_collection(node));
	fold(Get_get_value(node));
}

void fold_Slice(struct Node *const node) {
	fold(Slice_get_collection(node));
	fold(Slice_get_start(node));
	fold(Slice_get_end(node));
}

void fold_ListComp(struct Node *const node) {
	FOR_CHILDREN(i, child, node) {
		if (child->nodetype != N_LETITER) {
			fold(child);
		}
	}
}

void fold_TableComp(struct Node *const node) {
	FOR_CHILDREN(i, child, node) {
		if (child->nodetype != N_LETITER) {
			fold(child);
		}
	}
}

void fold_TriOp(struct Node *const node) {
	FOR_CHILDREN(i, child, node) {
		fold(child);
	}
}

static void make_float(struct Node *const node, yasl_float val) {
	node->nodetype = N_FLOAT;
	node->value.dval = val;
	node->children_len = 0;
}

static void make_int(struct Node *const node, yasl_int val) {
	node->nodetype = N_INT;
	node->value.ival = val;
	node->children_len = 0;
}

static void make_bool(struct Node *const node, int val) {
	node->nodetype = N_BOOL;
	node->value.ival = val;
	node->children_len = 0;
}

#define FOLD_BINOP_INT_INT(left, right, op, type) \
make_##type(node, (left)->value.ival op (right)->value.ival);\
node_del(left);\
node_del(right);

#define FOLD_BINOP_FLOAT_INT(left, right, op, type) \
make_##type(node, (left)->value.dval op (yasl_float)(right)->value.ival);\
node_del(left);\
node_del(right);

#define FOLD_BINOP_INT_FLOAT(left, right, op, type) \
make_##type(node, (yasl_float)(left)->value.ival op (right)->value.dval);\
node_del(left);\
node_del(right);

#define FOLD_BINOP_FLOAT_FLOAT(left, right, op, type) \
make_##type(node, (left)->value.dval op (right)->value.dval);\
node_del(left);\
node_del(right);

static void fold_BinOp(struct Node *const node) {
	fold(node->children[0]);
	fold(node->children[1]);
	struct Node *left = node->children[0];
	struct Node *right = node->children[1];
	if (left->nodetype == N_INT && right->nodetype == N_INT) {
		switch (node->value.type) {
		case T_BAR:
			FOLD_BINOP_INT_INT(left, right, |, int);
			break;
		case T_CARET:
			FOLD_BINOP_INT_INT(left, right, ^, int);
			break;
		case T_AMP:
			FOLD_BINOP_INT_INT(left, right, &, int);
			break;
		case T_AMPCARET:
			FOLD_BINOP_INT_INT(left, right, &~, int);
			break;
		case T_DEQ:
		case T_TEQ:
			FOLD_BINOP_INT_INT(left, right, ==, bool);
			break;
		case T_BANGEQ:
		case T_BANGDEQ:
			FOLD_BINOP_INT_INT(left, right, !=, bool);
			break;
		case T_GT:
			FOLD_BINOP_INT_INT(left, right, >, bool);
			break;
		case T_GTEQ:
			FOLD_BINOP_INT_INT(left, right, >=, bool);
			break;
		case T_LT:
			FOLD_BINOP_INT_INT(left, right, <, bool);
			break;
		case T_LTEQ:
			FOLD_BINOP_INT_INT(left, right, <=, bool);
			break;
		case T_TILDE:
			// TODO
			break;
		case T_DGT:
			FOLD_BINOP_INT_INT(left, right, >>, int);
			break;
		case T_DLT:
			FOLD_BINOP_INT_INT(left, right, <<, int);
			break;
		case T_PLUS:
			FOLD_BINOP_INT_INT(left, right, +, int);
			break;
		case T_MINUS:
			FOLD_BINOP_INT_INT(left, right, -, int);
			break;
		case T_STAR:
			FOLD_BINOP_INT_INT(left, right, *, int);
			break;
		case T_SLASH:
			make_float(node, (yasl_float)left->value.ival / (yasl_float)right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_DSLASH:
			if (right->value.ival != 0) {
				FOLD_BINOP_INT_INT(left, right, /, int);
			}
			break;
		case T_MOD:
			if (right->value.ival != 0) {
				FOLD_BINOP_INT_INT(left, right, %, int);
			}
			break;
		case T_DSTAR:
			break;
		default:
			break;
		}
	} else if (left->nodetype == N_FLOAT && right->nodetype == N_FLOAT) {
		switch (node->value.type) {
		case T_DEQ:
		case T_TEQ:
			FOLD_BINOP_FLOAT_FLOAT(left, right, ==, bool);
			break;
		case T_BANGEQ:
		case T_BANGDEQ:
			FOLD_BINOP_FLOAT_FLOAT(left, right, !=, bool);
			break;
		case T_GT:
			FOLD_BINOP_FLOAT_FLOAT(left, right, >, bool);
			break;
		case T_GTEQ:
			FOLD_BINOP_FLOAT_FLOAT(left, right, >=, bool);
			break;
		case T_LT:
			FOLD_BINOP_FLOAT_FLOAT(left, right, <, bool);
			break;
		case T_LTEQ:
			FOLD_BINOP_FLOAT_FLOAT(left, right, <=, bool);
			break;
		case T_TILDE:
			// TODO
			break;
		case T_PLUS:
			FOLD_BINOP_FLOAT_FLOAT(left, right, +, float);
			break;
		case T_MINUS:
			FOLD_BINOP_FLOAT_FLOAT(left, right, -, float);
			break;
		case T_STAR:
			FOLD_BINOP_FLOAT_FLOAT(left, right, *, float);
			break;
		case T_SLASH:
			FOLD_BINOP_FLOAT_FLOAT(left, right, /, float);
			break;
		case T_DSTAR:
			break;
		default:
			break;
		}
	} else if (left->nodetype == N_FLOAT && right->nodetype == N_INT) {
		switch (node->value.type) {
		case T_DEQ:
		case T_TEQ:
			FOLD_BINOP_FLOAT_INT(left, right, ==, bool);
			break;
		case T_BANGEQ:
		case T_BANGDEQ:
			FOLD_BINOP_FLOAT_INT(left, right, !=, bool);
			break;
		case T_GT:
			FOLD_BINOP_FLOAT_INT(left, right, >, bool);
			break;
		case T_GTEQ:
			FOLD_BINOP_FLOAT_INT(left, right, >=, bool);
			break;
		case T_LT:
			FOLD_BINOP_FLOAT_INT(left, right, <, bool);
			break;
		case T_LTEQ:
			FOLD_BINOP_FLOAT_INT(left, right, <=, bool);
			break;
		case T_TILDE:
			// TODO
			break;
		case T_PLUS:
			FOLD_BINOP_FLOAT_INT(left, right, +, float);
			break;
		case T_MINUS:
			FOLD_BINOP_FLOAT_INT(left, right, -, float);
			break;
		case T_STAR:
			FOLD_BINOP_FLOAT_INT(left, right, *, float);
			break;
		case T_SLASH:
			FOLD_BINOP_FLOAT_INT(left, right, /, float);
			break;
		case T_DSTAR:
			break;
		default:
			break;
		}
	} else if (left->nodetype == N_INT && right->nodetype == N_FLOAT) {
		switch (node->value.type) {
		case T_DEQ:
		case T_TEQ:
			FOLD_BINOP_INT_FLOAT(left, right, ==, bool);
			break;
		case T_BANGEQ:
		case T_BANGDEQ:
			FOLD_BINOP_INT_FLOAT(left, right, !=, bool);
			break;
		case T_GT:
			FOLD_BINOP_INT_FLOAT(left, right, >, bool);
			break;
		case T_GTEQ:
			FOLD_BINOP_INT_FLOAT(left, right, >=, bool);
			break;
		case T_LT:
			FOLD_BINOP_INT_FLOAT(left, right, <, bool);
			break;
		case T_LTEQ:
			FOLD_BINOP_INT_FLOAT(left, right, <=, bool);
			break;
		case T_TILDE:
			// TODO
			break;
		case T_PLUS:
			FOLD_BINOP_INT_FLOAT(left, right, +, float);
			break;
		case T_MINUS:
			FOLD_BINOP_INT_FLOAT(left, right, -, float);
			break;
		case T_STAR:
			FOLD_BINOP_INT_FLOAT(left, right, *, float);
			break;
		case T_SLASH:
			FOLD_BINOP_INT_FLOAT(left, right, /, float);
			break;
		case T_DSTAR:
			break;
		default:
			break;
		}
	}
}

void fold_UnOp(struct Node *const node) {
	fold(UnOp_get_expr(node));
	struct Node *expr = UnOp_get_expr(node);
	switch (expr->nodetype) {
	case N_INT:
		switch (node->value.type) {
		case T_PLUS:
			make_int(node, +expr->value.ival);
			node_del(expr);
			break;
		case T_MINUS:
			make_int(node, -expr->value.ival);
			node_del(expr);
			break;
		case T_BANG:
			make_bool(node, 0);
			node_del(expr);
			break;
		case T_CARET:
			make_int(node, ~expr->value.ival);
			node_del(expr);
			break;
		default:
			break;
		}
		break;
	case N_BOOL:
		switch (node->value.type) {
		case T_BANG:
			make_bool(node, !expr->value.ival);
			node_del(expr);
			break;
		default:
			break;
		}
		break;
	case N_FLOAT:
		switch (node->value.type) {
		case T_PLUS:
			make_float(node, +expr->value.dval);
			node_del(expr);
			break;
		case T_MINUS:
			make_float(node, -expr->value.dval);
			node_del(expr);
			break;
		case T_BANG:
			make_bool(node, 0);  // NOTE: safe because we don't have NaN literals
			node_del(expr);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

}

void fold_Assign(struct Node *const node) {
	fold(Assign_get_expr(node));
}

void fold_List(struct Node *const node) {
	FOR_CHILDREN(i, child, node) {
		fold(child);
	}
}

void fold_Table(struct Node *const node) {
	FOR_CHILDREN(i, child, node) {
		fold(child);
	}
}

void fold(struct Node *const node) {
	switch (node->nodetype) {
	case N_EXPRSTMT:
		fold_ExprStmt(node);
		break;
	case N_BLOCK:
		fold_Block(node);
		break;
	case N_BODY:
		fold_Body(node);
		break;
	case N_CALL:
		fold_Call(node);
		break;
	case N_MCALL:
		fold_MethodCall(node);
		break;
	case N_SET:
		fold_Set(node);
		break;
	case N_GET:
		fold_Get(node);
		break;
	case N_SLICE:
		fold_Slice(node);
		break;
	case N_LISTCOMP:
		fold_ListComp(node);
		break;
	case N_TABLECOMP:
		fold_TableComp(node);
		break;
	case N_TRIOP:
		fold_TriOp(node);
		break;
	case N_BINOP:
		fold_BinOp(node);
		break;
	case N_UNOP:
		fold_UnOp(node);
		break;
	case N_ASSIGN:
		fold_Assign(node);
		break;
	case N_LIST:
		fold_List(node);
		break;
	case N_TABLE:
		fold_Table(node);
		break;
	default:
		break;
	}
}
