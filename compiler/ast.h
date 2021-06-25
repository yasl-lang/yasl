#ifndef YASL_AST_H_
#define YASL_AST_H_

#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "yapp.h"
#include "yasl_conf.h"

#define X(name, E, ...) E,
// NOTE: _MUST_ keep this up to date with the jumptable in compiler.c and the jumptable in middleend.c
enum NodeType {
#include "nodetype.x"
};

#undef X

struct BinOpNode {
	enum Token op;
	struct Node *left;
	struct Node *right;
};

struct UnOpNode {
	enum Token op;
	struct Node *expr;
};

struct TriOpNode {
	struct Node *left;
	struct Node *middle;
	struct Node *right;
};

struct Node1 {
	struct Node *child;
};

struct Node2 {
	struct Node *children[2];
};

struct Node3 {
	struct Node *children[3];
};

struct BodyNode {
	size_t num_children;
	struct Node *children[];
};

struct Node {
	struct Node *next;
	enum NodeType nodetype;
	union {
		struct {
			char *str;
			size_t str_len;
		} sval;
		yasl_int ival;
		yasl_float dval;
		enum Token type;
		struct UnOpNode unop;
		struct BinOpNode binop;
	} value;
	size_t line;
	size_t children_len;
	struct Node *children[];
};

struct Parser;

void body_append(struct Parser *parser, struct Node **node, struct Node *const child);
struct Node *body_last(struct Node *node);

struct Node *node_clone(const struct Node *const node);

bool will_var_expand(struct Node *node);

#define FOR_CHILDREN(i, child, node) struct Node *child;\
for (size_t i = 0; i < (node)->children_len; i++ ) if (child = (node)->children[i], child != NULL)

#define DECL_NODE(...) YAPP_EXPAND(YAPP_CHOOSE6(__VA_ARGS__, DECL_NODE4, DECL_NODE3, DECL_NODE2, DECL_NODE1, DECL_NODE0)(__VA_ARGS__))
#define DECL_STR_NODE(...) YAPP_EXPAND(YAPP_CHOOSE6(__VA_ARGS__, DECL_STR_NODE4, DECL_STR_NODE3, DECL_STR_NODE2, DECL_STR_NODE1, DECL_STR_NODE0)(__VA_ARGS__))
#define DECL_INT_NODE(...) YAPP_EXPAND(YAPP_CHOOSE6(__VA_ARGS__, DECL_INT_NODE4, DECL_INT_NODE3, DECL_INT_NODE2, DECL_INT_NODE1, DECL_INT_NODE0)(__VA_ARGS__))
#define DECL_ZSTR_NODE(...) YAPP_EXPAND(YAPP_CHOOSE6(__VA_ARGS__, DECL_ZSTR_NODE4, DECL_ZSTR_NODE3, DECL_ZSTR_NODE2, DECL_ZSTR_NODE1, DECL_ZSTR_NODE0)(__VA_ARGS__))

#define DECL_NODE0(name, E) \
struct Node *new_##name(struct Parser *parser, size_t line);\

#define DECL_NODE1(name, E, a) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const size_t line);\
struct Node *name##_get_##a(const struct Node *const node);

#define DECL_NODE2(name, E, a, b) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, const size_t line);\
struct Node *name##_get_##a(const struct Node *const node);\
struct Node *name##_get_##b(const struct Node *const node);

#define DECL_NODE3(name, E, a, b, c) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, const struct Node *const c, const size_t line);\
struct Node *name##_get_##a(const struct Node *const node);\
struct Node *name##_get_##b(const struct Node *const node);\
struct Node *name##_get_##c(const struct Node *const node);

#define DECL_NODE4(name, E, a, b, c, d) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, const struct Node *const c, const struct Node *const d, const size_t line);\
struct Node *name##_get_##a(const struct Node *const node);\
struct Node *name##_get_##b(const struct Node *const node);\
struct Node *name##_get_##c(const struct Node *const node);\
struct Node *name##_get_##d(const struct Node *const node);

#define DECL_ZSTR_NODE0(name, E) \
struct Node *new_##name(struct Parser *parser, char *str, const size_t line);

#define DECL_ZSTR_NODE1(name, E, a) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, char *str, const size_t line);

#define DECL_ZSTR_NODE2(name, E, a, b) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, char *str, const size_t line);\
struct Node *name##_get_##a(const struct Node *const node);\
struct Node *name##_get_##b(const struct Node *const node);

#define DECL_INT_NODE2(name, E, a, b) \
struct Node *new_##name(struct Parser *parser, const struct Node *const a, const struct Node *const b, const size_t n, const size_t line);\
struct Node *name##_get_##a(const struct Node *const node);\
struct Node *name##_get_##b(const struct Node *const node);

DECL_NODE(ExprStmt, N_EXPRSTMT, expr)
DECL_NODE(Block, N_BLOCK, block)
DECL_NODE(Return, N_RET, expr)
DECL_NODE(MultiReturn, N_MULTIRET, exprs)
DECL_NODE(Export, N_EXPORT, expr)
DECL_INT_NODE(Call, N_CALL, params, object)
DECL_NODE(Set, N_SET, collection, key, value)
DECL_NODE(Get, N_GET, collection, value)
DECL_NODE(Slice, N_SLICE, collection, start, end)
DECL_NODE(ListComp, N_LISTCOMP, expr, iter, cond)
DECL_NODE(TableComp, N_TABLECOMP, expr, iter, cond)
DECL_NODE(ForIter, N_FORITER, iter, body)
DECL_NODE(While, N_WHILE, cond, body, post)
DECL_NODE(Break, N_BREAK)
DECL_NODE(Continue, N_CONT)
DECL_NODE(Match, N_MATCH, cond, patterns, guards, bodies)
DECL_NODE(If, N_IF, cond, then, el)

DECL_NODE(Echo, N_ECHO, expr)
DECL_NODE(Decl, N_DECL, lvals, rvals)
DECL_ZSTR_NODE(FnDecl, N_FNDECL, params, body)
DECL_NODE(Undef, N_UNDEF)
DECL_NODE(List, N_LIST, values)
DECL_NODE(Table, N_TABLE, values)
DECL_NODE(Assert, N_ASS, expr)
DECL_NODE(Body, N_BODY)

struct Node *new_VariadicContext(struct Node *const expr, const int expected);
struct Node *new_MethodCall(struct Parser *parser, const struct Node *const params, const struct Node *const object, char *value, size_t len, const size_t line);

DECL_ZSTR_NODE(LetIter, N_LETITER, collection)
DECL_ZSTR_NODE(Let, N_LET, expr)
DECL_ZSTR_NODE(Const, N_CONST, expr)
DECL_ZSTR_NODE(Assign, N_ASSIGN, expr)
DECL_ZSTR_NODE(Var, N_VAR)

struct Node *new_TriOp(struct Parser *parser, enum Token op, struct Node *left, struct Node *middle, struct Node *right, const size_t line);
struct Node *new_BinOp(struct Parser *parser, enum Token op, struct Node *left, struct Node *right, const size_t line);
struct Node *new_UnOp(struct Parser *parser, enum Token op, struct Node *child, const size_t line);
struct Node *new_Float(struct Parser *parser, yasl_float val, const size_t line);
struct Node *new_Integer(struct Parser *parser, yasl_int val, const size_t line);
struct Node *new_Boolean(struct Parser *parser, int value, const size_t line);
struct Node *new_String(struct Parser *parser, char *value, size_t len, const size_t line);

size_t Body_get_len(const struct Node *const node);
struct Node *Comp_get_expr(const struct Node *const node);
struct Node *Decl_get_expr(const struct Node *const node);
char *Decl_get_name(const struct Node *const node);
char *MethodCall_get_name(const struct Node *const node);
struct Node *MethodCall_get_params(const struct Node *const node);
struct Node *MethodCall_get_object(const struct Node *const node);
struct Node *UnOp_get_expr(const struct Node *const node);
struct Node *BinOp_get_left(const struct Node *const node);
struct Node *BinOp_get_right(const struct Node *const node);
struct Node *TriOp_get_left(const struct Node *const node);
struct Node *TriOp_get_middle(const struct Node *const node);
struct Node *TriOp_get_right(const struct Node *const node);
struct Node *Assign_get_expr(const struct Node *const node);
struct Node *Comp_get_iter(const struct Node *const node);
struct Node *Comp_get_cond(const struct Node *const node);
struct Node *LetIter_get_collection(const struct Node *const node);
const char *String_get_str(const struct Node *const node);
size_t String_get_len(const struct Node *const node);
yasl_int Integer_get_int(const struct Node *const node);
yasl_float Float_get_float(const struct Node *const node);
bool Boolean_get_bool(const struct Node *const node);
char *Var_get_name(const struct Node *const node);

void node_del(struct Node *node);

#endif
