#include "middleend.h"

#include "compiler/ast.h"
#include "ast.h"

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

void make_float(struct Node *const node, double val) {
	node->nodetype = N_FLOAT;
	node->value.dval = val;
	node->children_len = 0;
}

void make_int(struct Node *const node, yasl_int val) {
	node->nodetype = N_INT;
	node->value.ival = val;
	node->children_len = 0;
}

void make_bool(struct Node *const node, int val) {
	node->nodetype = N_BOOL;
	node->value.ival = val;
	node->children_len = 0;
}

void fold_BinOp(struct Node *const node) {
	fold(node->children[0]);
	fold(node->children[1]);
	struct Node *left = node->children[0];
	struct Node *right = node->children[1];
	if (left->nodetype == N_INT && right->nodetype == N_INT) {
		switch (node->type) {
		case T_BAR:
			make_int(node, left->value.ival | right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_CARET:
			make_int(node, left->value.ival ^ right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_AMP:
			make_int(node, left->value.ival & right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_AMPCARET:
			make_int(node, left->value.ival & ~right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_DEQ:
			make_bool(node, left->value.ival == right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_TEQ:
			make_bool(node, left->value.ival == right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_BANGEQ:
			make_bool(node, left->value.ival != right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_BANGDEQ:
			make_bool(node, left->value.ival != right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_GT:
			make_bool(node, left->value.ival > right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_GTEQ:
			make_bool(node, left->value.ival >= right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_LT:
			make_bool(node, left->value.ival < right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_LTEQ:
			make_bool(node, left->value.ival <= right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_TILDE:
			/*
			size_t len = snprintf(NULL, 0, "%lld", (long long) left->value.ival);
			make_bool(node, left->value.ival == right->value.ival);
			node_del(left);
			node_del(right);
			*/
			break;
		case T_DGT:
			make_int(node, left->value.ival >> right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_DLT:
			make_int(node, left->value.ival << right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_PLUS:
			make_int(node, left->value.ival + right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_MINUS:
			make_int(node, left->value.ival - right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_STAR:
			make_int(node, left->value.ival * right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_SLASH:
			break;
		case T_DSLASH:
			if (right->value.ival != 0) {
				make_int(node, left->value.ival / right->value.ival);
				node_del(left);
				node_del(right);
			}
			break;
		case T_MOD:
			if (right->value.ival != 0) {
				make_int(node, left->value.ival % right->value.ival);
				node_del(left);
				node_del(right);
			}
			break;
		case T_DSTAR:
			break;
		default:
			break;
		}
	} else if (left->nodetype == N_FLOAT && right->nodetype == N_FLOAT) {
		switch (node->type) {
		case T_DEQ:
		case T_TEQ:
			make_bool(node, left->value.dval == right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_BANGEQ:
		case T_BANGDEQ:
			make_bool(node, left->value.dval != right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_GT:
			make_bool(node, left->value.dval > right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_GTEQ:
			make_bool(node, left->value.dval >= right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_LT:
			make_bool(node, left->value.dval < right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_LTEQ:
			make_bool(node, left->value.dval <= right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_TILDE:
			/*
			size_t len = snprintf(NULL, 0, "%lld", (long long) left->value.ival);
			make_bool(node, left->value.ival == right->value.ival);
			node_del(left);
			node_del(right);
			*/
			break;
		case T_PLUS:
			make_float(node, left->value.dval + right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_MINUS:
			make_float(node, left->value.dval - right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_STAR:
			make_float(node, left->value.dval * right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_SLASH:
			make_float(node, left->value.dval / right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_DSTAR:
			break;
		default:
			break;
		}
	} else if (left->nodetype == N_FLOAT && right->nodetype == N_INT) {

		switch (node->type) {
		case T_DEQ:
		case T_TEQ:
			make_bool(node, left->value.dval == right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_BANGEQ:
		case T_BANGDEQ:
			make_bool(node, left->value.dval != right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_GT:
			make_bool(node, left->value.dval > right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_GTEQ:
			make_bool(node, left->value.dval >= right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_LT:
			make_bool(node, left->value.dval < right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_LTEQ:
			make_bool(node, left->value.dval <= right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_TILDE:
			/*
			size_t len = snprintf(NULL, 0, "%lld", (long long) left->value.ival);
			make_bool(node, left->value.ival == right->value.ival);
			node_del(left);
			node_del(right);
			*/
			break;
		case T_PLUS:
			make_float(node, left->value.dval + right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_MINUS:
			make_float(node, left->value.dval - right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_STAR:
			make_float(node, left->value.dval * right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_SLASH:
			make_float(node, left->value.dval / right->value.ival);
			node_del(left);
			node_del(right);
			break;
		case T_DSTAR:
			break;
		default:
			break;
		}
	} else if (left->nodetype == N_INT && right->nodetype == N_FLOAT) {
		switch (node->type) {
		case T_DEQ:
		case T_TEQ:
			make_bool(node, left->value.ival == right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_BANGEQ:
		case T_BANGDEQ:
			make_bool(node, left->value.ival != right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_GT:
			make_bool(node, left->value.ival > right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_GTEQ:
			make_bool(node, left->value.ival >= right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_LT:
			make_bool(node, left->value.ival < right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_LTEQ:
			make_bool(node, left->value.ival <= right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_TILDE:
			/*
			size_t len = snprintf(NULL, 0, "%lld", (long long) left->value.ival);
			make_bool(node, left->value.ival == right->value.ival);
			node_del(left);
			node_del(right);
			*/
			break;
		case T_PLUS:
			make_float(node, left->value.ival + right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_MINUS:
			make_float(node, left->value.ival - right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_STAR:
			make_float(node, left->value.ival * right->value.dval);
			node_del(left);
			node_del(right);
			break;
		case T_SLASH:
			make_float(node, left->value.ival / right->value.dval);
			node_del(left);
			node_del(right);
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
		switch (node->type) {
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
		switch (node->type) {
		case T_BANG:
			make_bool(node, !expr->value.ival);
			node_del(expr);
			break;
		default:
			break;
		}
		break;
	case N_FLOAT:
		switch (node->type) {
		case T_PLUS:
			make_float(node, +expr->value.dval);
			node_del(expr);
			break;
		case T_MINUS:
			make_float(node, -expr->value.dval);
			node_del(expr);
			break;
		case T_BANG:
			make_bool(node, 0);
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

void fold_Var(struct Node *const node) {
	// pass
}

void fold_Undef(struct Node *const node) {
	// pass
}

void fold_Float(struct Node *const node) {
	// pass
}

void fold_Integer(struct Node *const node) {
	// pass
}

void fold_Boolean(struct Node *const node) {
	// pass
}

void fold_String(struct Node *const node) {
	// pass
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


static void (*jumptable[])(struct Node *const ) = {
	NULL,
	NULL,
	&fold_Body,
	NULL,
	NULL,
	NULL,
	&fold_Call,
	&fold_MethodCall,
	&fold_Set,
	&fold_Get,
	&fold_Slice,
	NULL,
	&fold_ListComp,
	&fold_TableComp,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&fold_TriOp,
	&fold_BinOp,
	&fold_UnOp,
	&fold_Assign,
	&fold_Var,
	&fold_Undef,
	&fold_Float,
	&fold_Integer,
	&fold_Boolean,
	&fold_String,
	&fold_List,
	&fold_Table
};

void fold(struct Node *const node) {

	jumptable[node->nodetype](node);
}
