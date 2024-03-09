#include "compiler.h"

#include <math.h>
#include <stdarg.h>
#include <interpreter/YASL_Object.h>

#include "YASL_Object.h"
#include "ast.h"
#include "data-structures/YASL_String.h"
#include "lexinput.h"
#include "opcode.h"
#include "yasl_error.h"
#include "yasl_include.h"

static void validate_stmt(struct Compiler *compiler, const struct Node *const node);
static void validate_expr(struct Compiler *compiler, const struct Node *const node);

YASL_FORMAT_CHECK static void compiler_print_err(struct Compiler *compiler, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	compiler->parser.lex.err.print(&compiler->parser.lex.err, fmt, args);
	va_end(args);
}

#define compiler_print_err_syntax(compiler, format, ...) compiler_print_err(compiler, "SyntaxError: " format, __VA_ARGS__)

#define compiler_print_err_undeclared_var(compiler, name, line) \
	compiler_print_err_syntax((compiler), "Undeclared variable %s (line %" PRI_SIZET ").\n", name, line)

#define compiler_print_err_const(compiler, name, line) \
	compiler_print_err_syntax((compiler), "Cannot assign to constant %s (line %" PRI_SIZET ").\n", name, line)

#define break_checkpoint(compiler)    ((compiler)->checkpoints.items[(compiler)->checkpoints.count-1])
#define continue_checkpoint(compiler) ((compiler)->checkpoints.items[(compiler)->checkpoints.count-2])
#define stacksize_checkpoint(compiler) ((compiler)->checkpoints.items[(compiler)->checkpoints.count-3])


void compiler_tables_del(struct Compiler *compiler) {
	DEL_TABLE(&compiler->seen_bindings);
	YASL_Table_del(compiler->strings);
}

static void compiler_buffers_del(const struct Compiler *const compiler) {
	YASL_ByteBuffer_del(compiler->buffer);
	YASL_ByteBuffer_del(compiler->header);
	YASL_ByteBuffer_del(compiler->code);
	YASL_ByteBuffer_del(compiler->lines);
}

void compiler_cleanup(struct Compiler *const compiler) {
	compiler_tables_del(compiler);
	scope_del(compiler->globals);
	scope_del(compiler->stack);
	env_del(compiler->params);
	parser_cleanup(&compiler->parser);
	compiler_buffers_del(compiler);
	free(compiler->checkpoints.items);
}

static void handle_error(struct Compiler *const compiler) {
	compiler->status = YASL_SYNTAX_ERROR;
}

static bool in_function(const struct Compiler *const compiler) {
	return compiler->params != NULL;
}

static inline void compiler_add_byte(struct Compiler *const compiler, unsigned char byte) {
	YASL_ByteBuffer_add_byte(compiler->buffer, byte);
}

static inline void compiler_add_int(struct Compiler *const compiler, yasl_int n) {
	YASL_ByteBuffer_add_int(compiler->buffer, n);
}

static inline void compiler_add_code_BB(struct Compiler *const compiler, unsigned char b1, unsigned char b2) {
	compiler_add_byte(compiler, b1);
	compiler_add_byte(compiler, b2);
}

static inline void compiler_add_code_BW(struct Compiler *const compiler, unsigned char b, yasl_int n) {
	compiler_add_byte(compiler, b);
	compiler_add_int(compiler, n);
}

static inline void compiler_add_code_BBW(struct Compiler *const compiler, unsigned char b1, unsigned char b2, yasl_int n) {
	compiler_add_code_BB(compiler, b1, b2);
	compiler_add_int(compiler, n);
}

static void enter_scope(struct Compiler *const compiler) {
	struct Scope **lval = in_function(compiler) ? &compiler->params->scope : &compiler->stack;
	*lval = scope_new(*lval);
}

static void exit_scope(struct Compiler *const compiler) {
	struct Scope **lval = in_function(compiler) ? &compiler->params->scope : &compiler->stack;
	struct Scope *tmp = *lval;
	 *lval = tmp->parent;
	size_t num_locals = scope_num_vars_cur_only(tmp);
	scope_del_cur_only(tmp);
	if (num_locals > 0) {
		compiler_add_byte(compiler, O_DECSP);
		// TODO: make sure `num_locals` is small enough.
		compiler_add_byte(compiler, (char)num_locals);
	}
}

static inline void enter_conditional_false(struct Compiler *const compiler, int64_t *const index) {
	compiler_add_code_BW(compiler, O_BRF_8, -1);
	*index = compiler->buffer->count - 8;
}

static inline void exit_conditional_false(const struct Compiler *const compiler, const int64_t *const index) {
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, (size_t) *index, compiler->buffer->count - *index - 8);
}

static void add_checkpoint(struct Compiler *const compiler, const size_t cp) {
	BUFFER_PUSH(size_t)(&compiler->checkpoints, cp);
}

static void rm_checkpoint(struct Compiler *const compiler) {
	BUFFER_POP(size_t)(&compiler->checkpoints);
}

int is_const(const int64_t value) {
	const uint64_t MASK = 0x8000000000000000;
	return (MASK & value) != 0;
}

static inline int64_t get_index(const int64_t value) {
	return is_const(value) ? ~value : value;
}

static struct Scope *get_scope_in_use(const struct Compiler *const compiler) {
	return in_function(compiler) ? compiler->params->scope : compiler->stack;
}

static size_t get_stacksize(const struct Compiler *const compiler) {
	return scope_len(get_scope_in_use(compiler));
}

static struct Env *get_nearest(struct Env *env, const char *const name) {
	while (!env_contains_cur_only(env, name)) {
		env = env->parent;
	}
	return env;
}

static void load_var_local(struct Compiler *const compiler, const struct Scope *scope, const char *const name) {
	int64_t index = get_index(scope_get(scope, name));
	compiler_add_code_BB(compiler, O_LLOAD, (unsigned char) index);
}

static void load_var_from_upval(struct Compiler *const compiler, const char *const name) {
	compiler->params->isclosure = true;
	yasl_int tmp = env_resolve_upval_index(compiler->params, compiler->stack, name);
	compiler_add_code_BB(compiler, O_ULOAD, (unsigned char) tmp);
}


static bool var_is_defined(struct Compiler *const compiler, const char *const name) {
	return env_contains(compiler->params, name) || scope_contains(compiler->stack, name) || scope_contains(compiler->globals, name);
}

// NOTE: Keep this in sync with `var_is_defined`, and add tests for `ifdef` if you change this.
static void load_var(struct Compiler *const compiler, const char *const name, const size_t line) {
	if (in_function(compiler) && env_contains_cur_only(compiler->params, name)) {   // fn-local var
		load_var_local(compiler, compiler->params->scope, name);
	} else if (env_contains(compiler->params, name)) {                         // closure over fn-local variable
		struct Env *curr = get_nearest(compiler->params, name);
		curr->usedinclosure = true;
		load_var_from_upval(compiler, name);
	} else if (in_function(compiler) && scope_contains(compiler->stack, name)) {    // closure over file-local var
		load_var_from_upval(compiler, name);
	} else if (scope_contains(compiler->stack, name)) {                        // file-local vars
		load_var_local(compiler, compiler->stack, name);
	} else if (scope_contains(compiler->globals, name)) {                      // global vars
		compiler_add_code_BW(compiler, O_GLOAD_8, YASL_Table_search_zstring_int(compiler->strings, name).value.ival);
	} else {
		compiler_print_err_undeclared_var(compiler, name, line);
		handle_error(compiler);
	}
}

static void store_var_cur_scope(struct Compiler *const compiler, const struct Scope *const scope, const char *const name, const size_t line) {
	int64_t index = scope_get(scope, name);
	if (is_const(index)) {
		compiler_print_err_const(compiler, name, line);
		handle_error(compiler);
		return;
	}
	compiler_add_code_BB(compiler, O_LSTORE, (unsigned char)index);
}

static void store_var_in_upval(struct Compiler *const compiler, const char *const name) {
	compiler->params->isclosure = true;
	yasl_int index = env_resolve_upval_index(compiler->params, compiler->stack, name);
	compiler_add_code_BB(compiler, O_USTORE, (unsigned char)index);
}

static void store_var(struct Compiler *const compiler, const char *const name, const size_t line) {
	if (in_function(compiler) && env_contains_cur_only(compiler->params, name)) { // fn-local variable
		store_var_cur_scope(compiler, compiler->params->scope, name, line);
	} else if (env_contains(compiler->params, name)) {                            // closure over fn-local variable
		struct Env *curr = get_nearest(compiler->params, name);
		curr->usedinclosure = true;
		int64_t index = scope_get(curr->scope, name);
		if (is_const(index))
			goto handle_const_err;
		store_var_in_upval(compiler, name);
	} else if (in_function(compiler) && scope_contains(compiler->stack, name)) {  // closure over file-local var
		int64_t index = scope_get(compiler->stack, name);
		if (is_const(index))
			goto handle_const_err;
		store_var_in_upval(compiler, name);
	} else if (scope_contains(compiler->stack, name)) {                           // file-local vars
		store_var_cur_scope(compiler, compiler->stack, name, line);
	} else if (scope_contains(compiler->globals, name)) {                         // global vars
		int64_t index = scope_get(compiler->globals, name);
		if (is_const(index))
			goto handle_const_err;
		compiler_add_code_BW(compiler, O_GSTORE_8, YASL_Table_search_zstring_int(compiler->strings, name).value.ival);
	} else {
		compiler_print_err_undeclared_var(compiler, name, line);
		handle_error(compiler);
	}
	return;

	handle_const_err:
	compiler_print_err_const(compiler, name, line);
	handle_error(compiler);
}

static int contains_var_in_current_scope(const struct Compiler *const compiler, const char *const name) {
	return in_function(compiler) ?
	       scope_contains_cur_only(compiler->params->scope, name) :
	       compiler->stack ?
	       scope_contains_cur_only(compiler->stack, name) :
	       scope_contains_cur_only(compiler->globals, name);
}

static int contains_var(const struct Compiler *const compiler, const char *const name) {
	if (scope_contains(compiler->stack, name)) return true;
	if (env_contains(compiler->params, name)) return true;
	return scope_contains_cur_only(compiler->globals, name);
}

static void decl_var(struct Compiler *const compiler, const char *const name, const size_t line) {
	const size_t name_len = strlen(name);
	struct Scope *scope = get_scope_in_use(compiler);
	if (scope) {
		int64_t index = scope_decl_var(scope, name);
		if (index > 255) {
			compiler_print_err_syntax(compiler, "Too many variables in current scope (line %" PRI_SIZET ").\n",  line);
			handle_error(compiler);
		}
	} else {
		compiler_intern_string(compiler, name, name_len);
		scope_decl_var(compiler->globals, name);
	}
}

static void make_const(const struct Compiler * const compiler, const char *const name) {
	struct Scope *scope = get_scope_in_use(compiler);
	if (scope) scope_make_const(scope, name);
	else scope_make_const(compiler->globals, name);
}

static unsigned char *return_bytes(const struct Compiler *const compiler) {
	if (compiler->status) return NULL;

	YASL_ByteBuffer_rewrite_int_fast(compiler->header, 0, compiler->header->count);
	YASL_ByteBuffer_rewrite_int_fast(compiler->header, 8, compiler->code->count + compiler->header->count + 1);
	YASL_ByteBuffer_rewrite_int_fast(compiler->header, 16, compiler->strings->count);

	YASL_ByteBuffer_add_vint(compiler->lines, compiler->code->count);
	YASL_BYTECODE_DEBUG_LOG("%s\n", "header");
	for (size_t i = 0; i < compiler->header->count; i++) {
		if (i % 16 == 15)
			YASL_BYTECODE_DEBUG_LOG("%02x\n", compiler->header->items[i]);
		else
			YASL_BYTECODE_DEBUG_LOG("%02x ", compiler->header->items[i]);
	}
	YASL_BYTECODE_DEBUG_LOG("\n%s\n", "entry point");
	for (size_t i = 0; i < compiler->code->count; i++) {
		if (i % 16 == 15)
			YASL_BYTECODE_DEBUG_LOG("%02x\n", compiler->code->items[i]);
		else
			YASL_BYTECODE_DEBUG_LOG("%02x ", compiler->code->items[i]);
	}
	YASL_BYTECODE_DEBUG_LOG("%02x\n", O_HALT);
	YASL_BYTECODE_DEBUG_LOG("%s\n", "lines");
	for (size_t i = 0; i < compiler->lines->count; i++) {
		if (i % 16 == 15)
			YASL_BYTECODE_DEBUG_LOG("%02x\n", compiler->lines->items[i]);
		else
			YASL_BYTECODE_DEBUG_LOG("%02x ", compiler->lines->items[i]);
	}
	YASL_BYTECODE_DEBUG_LOG("%s", "\n");

	fflush(stdout);
	unsigned char *bytecode = (unsigned char *) malloc(
		compiler->code->count + compiler->header->count + 1 + compiler->lines->count);    // NOT OWN
	memcpy(bytecode, compiler->header->items, compiler->header->count);
	memcpy(bytecode + compiler->header->count, compiler->code->items, compiler->code->count);
	bytecode[compiler->code->count + compiler->header->count] = O_HALT;
	memcpy(bytecode + compiler->code->count + 1 + compiler->header->count, compiler->lines->items, compiler->lines->count);
	return bytecode;
}

#define X(name, ...) static int visit_##name(struct Compiler *const compiler, const struct Node *const node, int stack_height);
#include "exprnodetype.x"
#undef X

static int visit_expr(struct Compiler *const compiler, const struct Node *const node, int stack_height);
static void visit_patt(struct Compiler *const compiler, const struct Node *const node);
static void visit_stmt(struct Compiler *const compiler, const struct Node *const node);

unsigned char *compile(struct Compiler *const compiler) {
	struct Node *node;
	gettok(&compiler->parser.lex);
	enter_scope(compiler);
	while (!peof(&compiler->parser)) {
		if (peof(&compiler->parser)) break;
		node = parse(&compiler->parser);
		if (compiler->parser.status) {
			compiler->status |= compiler->parser.status;
			return NULL;
		}
		eattok(&compiler->parser, T_SEMI);
		if (compiler->parser.status) {
			compiler->status |= compiler->parser.status;
			return NULL;
		}
		visit_stmt(compiler, node);
		YASL_ByteBuffer_extend(compiler->code, compiler->buffer->items, compiler->buffer->count);
		compiler->buffer->count = 0;
	}
	exit_scope(compiler);

	return return_bytes(compiler);
}

unsigned char *compile_REPL(struct Compiler *const compiler) {
	struct Node *node;
	gettok(&compiler->parser.lex);
	while (!peof(&compiler->parser)) {
		if (peof(&compiler->parser)) break;
		node = parse(&compiler->parser);
		eattok(&compiler->parser, T_SEMI);
		compiler->status |= compiler->parser.status;
		if (!compiler->parser.status) {
			if (peof(&compiler->parser) && node->nodetype == N_EXPRSTMT) {
				node->nodetype = N_ECHO;
			}
			visit_stmt(compiler, node);
			YASL_ByteBuffer_extend(compiler->code, compiler->buffer->items, compiler->buffer->count);
			compiler->buffer->count = 0;
		}
	}

	return return_bytes(compiler);
}

static void visit_Body(struct Compiler *const compiler, const struct Node *const node) {
	FOR_CHILDREN(i, child, node) {
		visit_stmt(compiler, child);
	}
}

static int visit_Exprs(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	FOR_CHILDREN(i, child, node) {
			visit_expr(compiler, child, stack_height + (int)i);
	}
	return stack_height + 1;
}

static void visit_ExprStmt(struct Compiler *const compiler, const struct Node *const node) {
	const struct Node *const expr = ExprStmt_get_expr(node);
	switch (expr->nodetype) {
	case N_STR:
	case N_INT:
	case N_FLOAT:
	case N_BOOL:
	case N_UNDEF:
		return;
	case N_VAR:
		validate_expr(compiler, expr);
		return;
	default:
		visit_expr(compiler, expr, (int)get_stacksize(compiler));
		compiler_add_byte(compiler, O_POP);
	}
}

static int return_op(struct Compiler *compiler) {
	return compiler->params->usedinclosure ? O_CRET : O_RET;
}

static int visit_FnDecl(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_UNUSED(stack_height);
	compiler->params = env_new(compiler->params);

	enter_scope(compiler);

	FOR_CHILDREN(i, child, FnDecl_get_params(node)) {
		if (child->nodetype == N_VARGS) {
			decl_var(compiler, "...", child->line);
		} else {
			decl_var(compiler, Decl_get_name(child), child->line);
			if (child->nodetype == N_CONST) {
				make_const(compiler, Decl_get_name(child));
			}
		}
	}

	compiler_add_code_BW(compiler, O_FCONST, -1);

	size_t old_size = compiler->buffer->count;

	struct Node *body = FnDecl_get_body(node);
	bool is_variadic = body->children_len > 0 && body->children[0]->nodetype == N_COLLECTRESTPARAMS;

	size_t num_params = Body_get_len(FnDecl_get_params(node)) - (int)is_variadic;
	// TODO: verfiy that number of params is small enough. (same for the other casts below.)
	compiler_add_byte(compiler, (unsigned char)(is_variadic ? ~num_params : num_params));
	visit_Body(compiler, FnDecl_get_body(node));

	// Implicit return at the end of the function.
	compiler_add_code_BB(compiler, return_op(compiler), (unsigned char)get_stacksize(compiler));

	exit_scope(compiler);

	size_t new_size = compiler->buffer->count;
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, old_size - sizeof(yasl_int), new_size - old_size);

	if (compiler->params->isclosure) {
		compiler->buffer->items[old_size - sizeof(yasl_int) - 1] = O_CCONST;
		const size_t count = compiler->params->upval_indices.count;
		compiler_add_byte(compiler, (unsigned char) count);
		const size_t start = compiler->buffer->count;
		// TODO what's below is wrong. We need to get the right bytes for the upvals.
		for (size_t i = 0; i < count; i++) {
			compiler_add_byte(compiler, 0);
		}
		FOR_TABLE(i, item, &compiler->params->upval_indices) {
			int64_t index = item->value.value.ival;
			int64_t value = YASL_Table_search(&compiler->params->upval_values, item->key).value.ival;
			compiler->buffer->items[start + index] = value;
		}
	}

	struct Env *tmp = compiler->params->parent;
	compiler->params->parent = NULL;
	env_del(compiler->params);
	compiler->params = tmp;

	return stack_height + 1;
}

static void visit_CollectRestParams(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	YASL_ASSERT(in_function(compiler), "ast gen should ensure this is only called from inside a function.");
	compiler_add_byte(compiler, O_COLLECT_REST_PARAMS);
}

static int visit_Call(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	visit_expr(compiler, Call_get_object(node), stack_height);
	compiler_add_code_BB(compiler, O_INIT_CALL, (unsigned char)node->value.ival);
	visit_expr(compiler, Call_get_params(node), stack_height + 1);
	compiler_add_byte(compiler, O_CALL);

	return stack_height + 1;
}

static int visit_MethodCall(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	char *str = MethodCall_get_name(node);
	size_t len = strlen(str);
	visit_expr(compiler, MethodCall_get_object(node), stack_height);

	yasl_int index = compiler_intern_string(compiler, str, len);

	compiler_add_code_BBW(compiler, O_INIT_MC, (unsigned char)node->value.sval.str_len, index);

	visit_expr(compiler, MethodCall_get_params(node), stack_height + 2);  // +2 for function and object
	compiler_add_byte(compiler, O_CALL);

	return stack_height + 1;
}

static void visit_Return(struct Compiler *const compiler, const struct Node *const node) {
	if (!in_function(compiler)) {
		compiler_print_err_syntax(compiler, "`return` outside of function (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}
	visit_expr(compiler, Return_get_exprs(node), (int)get_stacksize(compiler));
	compiler_add_code_BB(compiler, return_op(compiler), (unsigned char)get_stacksize(compiler));
}

static void visit_Export(struct Compiler *const compiler, const struct Node *const node) {
	if (in_function(compiler) || compiler->stack && compiler->stack->parent) {
		compiler_print_err_syntax(compiler, "`export` statement must be at top level of module (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}
	visit_expr(compiler, Export_get_expr(node), (int)get_stacksize(compiler));
	compiler_add_byte(compiler, O_EXPORT);
}

static void visit_Set(struct Compiler *const compiler, const struct Node *const node) {
	const int size = (int)get_stacksize(compiler);
	visit_expr(compiler, Set_get_collection(node), size);
	visit_expr(compiler, Set_get_key(node), size + 1);
	visit_expr(compiler, Set_get_value(node), size + 2);
	compiler_add_byte(compiler, O_SET);
}

static int visit_Get(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	visit_expr(compiler, Get_get_collection(node), stack_height);
	visit_expr(compiler, Get_get_value(node), stack_height + 1);
	compiler_add_byte(compiler, O_GET);

	return stack_height + 1;
}

static int visit_Slice(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	visit_expr(compiler, Slice_get_collection(node), stack_height);
	visit_expr(compiler, Slice_get_start(node), stack_height + 1);
	visit_expr(compiler, Slice_get_end(node), stack_height + 2);
	compiler_add_byte(compiler, O_SLICE);

	return stack_height + 1;
}

static void visit_Block(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);
	visit_stmt(compiler, Block_get_block(node));
	exit_scope(compiler);
}

static inline void branch_back(struct Compiler *const compiler, int64_t index) {
	compiler_add_byte(compiler, O_BR_8);
	compiler_add_int(compiler, index - compiler->buffer->count - 8);
}

static void visit_Comp_cond(struct Compiler *const compiler, const struct Node *const cond, const struct Node *const expr, unsigned char byte, int stack_height) {
	(void) byte;
	if (cond) {
		int64_t index_third;
		visit_expr(compiler, cond, stack_height);
		enter_conditional_false(compiler, &index_third);

		visit_expr(compiler, expr, stack_height);
		compiler_add_byte(compiler, byte);

		exit_conditional_false(compiler, &index_third);
	} else {
		visit_expr(compiler, expr, stack_height);
		compiler_add_byte(compiler, byte);
	}
}

static void visit_Comp(struct Compiler *const compiler, const struct Node *const node, unsigned char collection_type, unsigned char byte, int stack_height) {
	enter_scope(compiler);
	struct Node *expr = Comp_get_expr(node);
	struct Node *cond = Comp_get_cond(node);
	struct Node *iter = Comp_get_iter(node);

	struct Node *collection = LetIter_get_collection(iter);

	char *name = iter->value.sval.str;

	visit_expr(compiler, collection, stack_height);

	const size_t stack_size_before = get_stacksize(compiler);
	compiler_add_byte(compiler, O_INITFOR);
	compiler_add_byte(compiler, O_END);
	compiler_add_byte(compiler, collection_type);
	decl_var(compiler, name, iter->line);
	compiler_add_byte(compiler, O_END);
	compiler_add_byte(compiler, O_MOVEDOWN_FP);
	compiler_add_byte(compiler, (unsigned char)stack_size_before);

	int64_t index_start = compiler->buffer->count;

	compiler_add_byte(compiler, O_ITER_1);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, name, iter->line);

	visit_Comp_cond(compiler, cond, expr, byte, stack_height + 2);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	compiler_add_byte(compiler, O_ENDCOMP);
	compiler_add_byte(compiler, (unsigned char)stack_size_before);

	size_t curr = compiler->buffer->count;
	exit_scope(compiler);
	compiler->buffer->count = curr;
}

static int visit_ListComp(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	visit_Comp(compiler, node, O_NEWLIST, O_LIST_PUSH, stack_height);

	return stack_height + 1;
}

static int visit_TableComp(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	visit_Comp(compiler, node, O_NEWTABLE, O_TABLE_SET, stack_height);

	return stack_height + 1;
}

static void visit_ForIter(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);

	struct Node *iter = ForIter_get_iter(node);
	struct Node *body = ForIter_get_body(node);

	struct Node *collection = LetIter_get_collection(iter);
	char *name = iter->value.sval.str;

	visit_expr(compiler, collection, (int)get_stacksize(compiler));

	compiler_add_byte(compiler, O_INITFOR);
	compiler_add_byte(compiler, O_END);
	decl_var(compiler, name, iter->line);

	add_checkpoint(compiler, get_stacksize(compiler));
	size_t index_start = compiler->buffer->count;
	add_checkpoint(compiler, index_start);

	compiler_add_byte(compiler, O_ITER_1);

	add_checkpoint(compiler, compiler->buffer->count);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, name, iter->line);

	visit_stmt(compiler, body);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	compiler_add_byte(compiler, O_ENDFOR);
	exit_scope(compiler);

	rm_checkpoint(compiler);
	rm_checkpoint(compiler);
	rm_checkpoint(compiler);
}

static void enter_jump(struct Compiler *const compiler, size_t *index) {
	compiler_add_code_BW(compiler, O_BR_8, -1);
	*index = compiler->buffer->count - 8;
}

static void exit_jump(struct Compiler *const compiler, const size_t *const index) {
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, *index, compiler->buffer->count - *index - 8);
}

static bool Node_istruthy(const struct Node *const node) {
	switch (node->nodetype)	{
	case N_INT:
		return true;
	case N_BOOL:
		return Boolean_get_bool(node);
	default:
		return false;
	}
}

static bool Node_isfalsey(const struct Node *const node) {
	switch (node->nodetype)	{
	case N_BOOL:
		return !Boolean_get_bool(node);
	case N_UNDEF:
		return true;
	default:
		return false;
	}
}

static void visit_While_false(struct Compiler *const compiler, const struct Node *const body, const struct Node *const post) {
	validate_stmt(compiler, body);
	if (post) validate_stmt(compiler, post);
}

static void visit_While(struct Compiler *const compiler, const struct Node *const node) {
	size_t index_start = compiler->buffer->count;

	struct Node *cond = While_get_cond(node);
	struct Node *body = While_get_body(node);
	struct Node *post = While_get_post(node);

	if (Node_isfalsey(cond)) {
		visit_While_false(compiler, body, post);
		return;
	}

	if (post) {
		size_t index;
		enter_jump(compiler, &index);
		index_start = compiler->buffer->count;
		visit_stmt(compiler, post);
		exit_jump(compiler, &index);
	}

	add_checkpoint(compiler, get_stacksize(compiler));
	add_checkpoint(compiler, index_start);

	visit_expr(compiler, cond, (int)get_stacksize(compiler));

	add_checkpoint(compiler, compiler->buffer->count);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	visit_stmt(compiler, body);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	rm_checkpoint(compiler);
	rm_checkpoint(compiler);
	rm_checkpoint(compiler);
}

static void visit_Break(struct Compiler *const compiler, const struct Node *const node) {
	if (compiler->checkpoints.count == 0) {
		compiler_print_err_syntax(compiler, "`break` outside of loop (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}
	compiler_add_byte(compiler, O_BCONST_F);
	branch_back(compiler, break_checkpoint(compiler));
}

static void visit_Continue(struct Compiler *const compiler, const struct Node *const node) {
	if (compiler->checkpoints.count == 0) {
		compiler_print_err_syntax(compiler, "`continue` outside of loop (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}

	size_t num_pops = get_stacksize(compiler) - stacksize_checkpoint(compiler);
	while (num_pops-- > 0)
		compiler_add_byte(compiler, O_POP);
	branch_back(compiler, continue_checkpoint(compiler));
}

yasl_int compiler_intern_string(struct Compiler *const compiler, const char *const str, const size_t len) {
	struct YASL_Object value = YASL_Table_search_string_int(compiler->strings, str, len);
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
		size_t index = compiler->strings->count;
		YASL_Table_insert_string_int(compiler->strings, str, len, index);
		YASL_ByteBuffer_add_byte(compiler->header, C_STR);
		YASL_ByteBuffer_add_int(compiler->header, len);
		YASL_ByteBuffer_extend(compiler->header, (unsigned char *) str, len);
		return index;
	}

	return value.value.ival;
}

yasl_int compiler_intern_float(struct Compiler *const compiler, const yasl_float val) {
	struct YASL_Object value = YASL_Table_search(compiler->strings, YASL_FLOAT(val));
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching float");
		yasl_int index = (yasl_int)compiler->strings->count;
		YASL_Table_insert(compiler->strings, YASL_FLOAT(val), YASL_INT(index));
		YASL_ByteBuffer_add_byte(compiler->header, C_FLOAT);
		YASL_ByteBuffer_add_float(compiler->header, val);

		return index;
	}

	return value.value.ival;
}

yasl_int compiler_intern_int(struct Compiler *const compiler, const yasl_int val) {
	struct YASL_Object value = YASL_Table_search(compiler->strings, YASL_INT(val));
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching integer");
		yasl_int index = (yasl_int)compiler->strings->count;
		YASL_Table_insert(compiler->strings, YASL_INT(val), YASL_INT(index));
		if (-(1 << 7) < val && val < (1 << 7)) {
			YASL_ByteBuffer_add_byte(compiler->header, C_INT_1);
			YASL_ByteBuffer_add_byte(compiler->header, (unsigned char) val);
		} else {
			YASL_ByteBuffer_add_byte(compiler->header, C_INT_8);
			YASL_ByteBuffer_add_int(compiler->header, val);
		}
		return index;
	}

	return value.value.ival;
}

static yasl_int intern_string(struct Compiler *const compiler, const struct Node *const node) {
	const char *const str = String_get_str(node);
	size_t len = String_get_len(node);

	return compiler_intern_string(compiler, str, len);
}

static void visit_UndefPattern(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	compiler_add_byte(compiler, P_UNDEF);
}

static void visit_BoolPattern(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, P_BOOL);
	compiler_add_byte(compiler, (unsigned char)(Boolean_get_bool(node)));
}

static void compiler_add_literal_pattern(struct Compiler *const compiler, const yasl_int index) {
	if (index < 128) {
		compiler_add_code_BB(compiler, P_LIT, (unsigned char)index);
	} else {
		compiler_add_code_BW(compiler, P_LIT8, index);
	}
}

static void visit_FloatPattern(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int index = compiler_intern_float(compiler, Float_get_float(node));
	compiler_add_literal_pattern(compiler, index);
}

static void visit_IntPattern(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int index = compiler_intern_int(compiler, Integer_get_int(node));
	compiler_add_literal_pattern(compiler, index);
}

static void visit_StringPattern(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int index = intern_string(compiler, node);
	compiler_add_literal_pattern(compiler, index);
}

static void visit_CollectionPattern(struct Compiler *const compiler, const struct Node *const node, unsigned char byte) {
	compiler_add_code_BW(compiler, byte, node->children_len);
	bool old = compiler->leftmost_pattern;
	FOR_CHILDREN(i, child, node) {
		visit_patt(compiler, child);
		compiler->leftmost_pattern = old;
	}
}

static void visit_TablePattern(struct Compiler *const compiler, const struct Node *const node) {
	visit_CollectionPattern(compiler, node, P_TABLE);
}

static void visit_ListPattern(struct Compiler *const compiler, const struct Node *const node) {
	visit_CollectionPattern(compiler, node, P_LS);
}

static void visit_VariadicTablePattern(struct Compiler *const compiler, const struct Node *const node) {
	visit_CollectionPattern(compiler, node, P_VTABLE);
}

static void visit_VariadicListPattern(struct Compiler *const compiler, const struct Node *const node) {
	visit_CollectionPattern(compiler, node, P_VLS);
}

static void visit_DeclPattern(struct Compiler *const compiler, const struct Node *const node, const bool isconst) {
	char *name = Decl_get_name(node);
	YASL_Table_insert_zstring_int(&compiler->seen_bindings, name, 1);
	if (!compiler->leftmost_pattern) {
		if (!contains_var_in_current_scope(compiler, name)) {
			compiler_print_err_syntax(compiler, "%s not bound on left side of | (line %" PRI_SIZET ").\n", name, node->line);
			handle_error(compiler);
			return;
		}
	} else {
		if (contains_var_in_current_scope(compiler, name)) {
			compiler_print_err_syntax(compiler, "Illegal rebinding of %s (line %" PRI_SIZET ").\n", name, node->line);
			handle_error(compiler);
			return;
		}
		decl_var(compiler, name, node->line);
		if (isconst) make_const(compiler, Decl_get_name(node));
	}

	compiler_add_byte(compiler, P_BIND);
	int64_t index = scope_get(get_scope_in_use(compiler), name);
	if (is_const(index) != isconst) {
		compiler_print_err_syntax(compiler, "%s must be bound with either `const` or `let` on both sides of | (line %" PRI_SIZET ").\n", name, node->line);
		handle_error(compiler);
		return;
	}

	compiler_add_byte(compiler, (unsigned char)get_index(index));
}

static void visit_LetPattern(struct Compiler *const compiler, const struct Node *const node) {
	visit_DeclPattern(compiler, node, false);
}

static void visit_ConstPattern(struct Compiler *const compiler, const struct Node *const node) {
	visit_DeclPattern(compiler, node, true);
}

static void visit_AnyPattern(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	compiler_add_byte(compiler, P_ANY);
}

static void visit_NotPattern(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, P_NOT);
	visit_patt(compiler, UnOp_get_expr(node));
}

static void visit_AltPattern(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, P_ALT);
	struct YASL_Table prev = compiler->seen_bindings;
	compiler->seen_bindings = NEW_TABLE();
	visit_patt(compiler, BinOp_get_left(node));

	if (compiler->status) {
		DEL_TABLE(&prev);
		return;
	}

	struct YASL_Table old = compiler->seen_bindings;
	compiler->seen_bindings = NEW_TABLE();
	compiler->leftmost_pattern = false;

	visit_patt(compiler, BinOp_get_right(node));

	if (compiler->status) {
		goto cleanup;
	}

	FOR_TABLE(i, item, &old) {
		struct YASL_Object val = YASL_Table_search(&compiler->seen_bindings, item->key);
		if (val.type == Y_END) {
			compiler_print_err_syntax(compiler, "%.*s not bound on right side of | (line %" PRI_SIZET ").\n", (int)YASL_String_len(item->key.value.sval), YASL_String_chars(item->key.value.sval), node->line);
			handle_error(compiler);
			goto cleanup;
		}
		YASL_Table_insert(&prev, item->key, YASL_INT(1));
	}
	
cleanup:
	DEL_TABLE(&old);
	DEL_TABLE(&compiler->seen_bindings);
	compiler->seen_bindings = prev;
}

static void visit_BoolTypePattern(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	compiler_add_byte(compiler, P_TYPE_BOOL);
}

static void visit_IntTypePattern(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	compiler_add_byte(compiler, P_TYPE_INT);
}

static void visit_FloatTypePattern(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	compiler_add_byte(compiler, P_TYPE_FLOAT);
}

static void visit_StringTypePattern(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	compiler_add_byte(compiler, P_TYPE_STR);
}

static void visit_ListTypePattern(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	compiler_add_byte(compiler, P_TYPE_LS);
}

static void visit_TableTypePattern(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(node);
	compiler_add_byte(compiler, P_TYPE_TABLE);
}

static void visit_Match_helper(struct Compiler *const compiler, const struct Node *const patterns, const struct Node *const guards, const struct Node *const bodies, size_t curr) {
	compiler_add_code_BW(compiler, O_MATCH, -1);
	size_t start = compiler->buffer->count;

	size_t vars = get_stacksize(compiler);
	enter_scope(compiler);
	compiler->leftmost_pattern = true;
	visit_patt(compiler, patterns->children[curr]);

	struct Node *guard = guards->children[curr];
	int64_t start_guard = 0;

	unsigned char bindings = (unsigned char) scope_num_vars_cur_only(get_scope_in_use(compiler));
	if (bindings) {
		compiler_add_code_BB(compiler, O_INCSP, bindings);
		if (guard) {
			compiler_add_code_BB(compiler, O_MOVEUP_FP, (unsigned char) vars);
			visit_expr(compiler, guard, (int)get_stacksize(compiler) + 1);
			enter_conditional_false(compiler, &start_guard);
			compiler_add_byte(compiler, O_POP);
		} else {
			compiler_add_code_BB(compiler, O_DEL_FP, (unsigned char) vars);
		}
	} else {
		if (guard) {
			visit_expr(compiler, guard, (int)get_stacksize(compiler) + 1);
			enter_conditional_false(compiler, &start_guard);
		}
		compiler_add_byte(compiler, O_POP);
	}

	// compiler_add_byte(compiler, O_POP);
	visit_stmt(compiler, bodies->children[curr]);
	exit_scope(compiler);

	curr++;
	size_t body_end = 0;
	if (patterns->children_len > curr) {
		enter_jump(compiler, &body_end);
	}

	if (guard) {
		YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, start_guard, compiler->buffer->count - start_guard - 8);
		if (bindings) {
			for (unsigned char i = bindings; i > 0; i--)
				compiler_add_byte(compiler, O_POP);
		}
	}
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, start - 8, compiler->buffer->count - start);

	if (patterns->children_len > curr) {
		visit_Match_helper(compiler, patterns, guards, bodies, curr);
		exit_jump(compiler, &body_end);
	}
}

static void visit_Match(struct Compiler *const compiler, const struct Node *const node) {
	struct Node *expr = Match_get_cond(node);
	struct Node *patterns = Match_get_patterns(node);
	struct Node *guards = Match_get_guards(node);
	struct Node *bodies = Match_get_bodies(node);
	visit_expr(compiler, expr, (int)get_stacksize(compiler));
	if (patterns->children_len == 0) {
		compiler_add_byte(compiler, O_POP);
		return;
	}

	visit_Match_helper(compiler, patterns, guards, bodies, 0);
}

static void visit_If_true(struct Compiler *const compiler, const struct Node *const then_br, const struct Node *const else_br) {
	visit_stmt(compiler, then_br);
	if (else_br) validate_stmt(compiler, else_br);
}

static void visit_If_false(struct Compiler *const compiler, const struct Node *const then_br, const struct Node *const else_br) {
	validate_stmt(compiler, then_br);
	if (else_br) visit_stmt(compiler, else_br);
}

static void visit_If(struct Compiler *const compiler, const struct Node *const node) {
	struct Node *cond = If_get_cond(node);
	struct Node *then_br = If_get_then(node);
	struct Node *else_br = If_get_el(node);

	if (Node_istruthy(cond)) {
		visit_If_true(compiler, then_br, else_br);
		return;
	}

	if (Node_isfalsey(cond)) {
		visit_If_false(compiler, then_br, else_br);
		return;
	}

	visit_expr(compiler, cond, (int)get_stacksize(compiler));

	int64_t index_then;
	enter_conditional_false(compiler, &index_then);
	visit_stmt(compiler, then_br);

	size_t index_else = 0;

	if (else_br) {
		enter_jump(compiler, &index_else);
	}


	exit_conditional_false(compiler, &index_then);

	if (else_br) {
		visit_stmt(compiler, else_br);
		exit_jump(compiler, &index_else);
	}
}

static bool vars_are_defined(struct Compiler *const compiler, const struct Node *const node) {
	switch (node->nodetype) {
	case N_UNOP:
		YASL_ASSERT(node->value.type == T_BANG, "parser should have generated a ! here.");
		return !vars_are_defined(compiler, UnOp_get_expr(node));
	/*
	case N_BINOP:
		YASL_ASSERT(node->value.type == T_DAMP || node->value.type == T_DBAR, "parser should have generated a & or | here.");
		if (node->value.type == T_DAMP) {
			return vars_are_defined(compiler, BinOp_get_left(node)) && vars_are_defined(compiler, BinOp_get_right(node));
		} else {
			return vars_are_defined(compiler, BinOp_get_left(node)) || vars_are_defined(compiler, BinOp_get_right(node));
		}
	 */
	case N_VAR:
		return var_is_defined(compiler, Var_get_name(node));
	default:
		YASL_UNREACHED();
	}
	return false;
}

static void visit_IfDef(struct Compiler *const compiler, const struct Node *const node) {
	struct Node *cond = IfDef_get_cond(node);
	struct Node *then_br = IfDef_get_then(node);
	struct Node *else_br = IfDef_get_el(node);

	if (vars_are_defined(compiler, cond)) {
		visit_stmt(compiler, then_br);
	} else if (else_br) {
		visit_stmt(compiler, else_br);
	}
}

static void visit_Echo(struct Compiler *const compiler, const struct Node *const node) {
	const int size = (int)get_stacksize(compiler);
	visit_expr(compiler, Echo_get_exprs(node), size);
	compiler_add_code_BB(compiler, O_ECHO, (char)get_stacksize(compiler));
}

static void declare_with_let_or_const(struct Compiler *const compiler, const struct Node *const node) {
	char *name = Decl_get_name(node);
	if (contains_var_in_current_scope(compiler, name)) {
		compiler_print_err_syntax(compiler, "Illegal redeclaration of %s (line %" PRI_SIZET ").\n", name, node->line);
		handle_error(compiler);
		return;
	}

	struct Node *expr = Decl_get_expr(node);
	if (expr &&
	    expr->nodetype == N_FNDECL &&
	    expr->value.sval.str != NULL) {
		decl_var(compiler, name, node->line);
		visit_expr(compiler, expr, (int)get_stacksize(compiler) - 1);
	} else {
		if (expr) visit_expr(compiler, expr, (int)get_stacksize(compiler));
		else compiler_add_byte(compiler, O_NCONST);

		decl_var(compiler, name, node->line);
	}

	struct Scope *scope = get_scope_in_use(compiler);

	if (!(scope_contains(scope, name))) {
		store_var(compiler, name, node->line);
	}
}

static void visit_Let(struct Compiler *const compiler, const struct Node *const node) {
	declare_with_let_or_const(compiler, node);
}

static void visit_Const(struct Compiler *const compiler, const struct Node *const node) {
	declare_with_let_or_const(compiler, node);
	make_const(compiler, Decl_get_name(node));
}

static void visit_Decl(struct Compiler *const compiler, const struct Node *const node) {
	visit_expr(compiler, Decl_get_rvals(node), (int)get_stacksize(compiler));

	FOR_CHILDREN(i, child, Decl_get_lvals(node)) {
		const char *name = child->value.sval.str;
		if (child->nodetype == N_ASSIGN) {
			if (!contains_var(compiler, name)) {
				compiler_print_err_undeclared_var(compiler, name, node->line);
				handle_error(compiler);
				return;
			}
			compiler_add_code_BB(compiler, O_MOVEUP_FP, (unsigned char)get_stacksize(compiler));
			store_var(compiler, name, node->line);
		} else if (child->nodetype == N_SET) {
			const int offset = (int)(Decl_get_lvals(node)->children_len - i);
			visit_expr(compiler, Set_get_collection(child), (int)get_stacksize(compiler) + offset);
			visit_expr(compiler, Set_get_key(child), (int)get_stacksize(compiler) + offset + 1);
			compiler_add_code_BB(compiler, O_MOVEUP_FP, (unsigned char)get_stacksize(compiler));
			compiler_add_byte(compiler, O_SET);
		} else {
			if (contains_var_in_current_scope(compiler, name)) {
				compiler_print_err_syntax(compiler, "Illegal redeclaration of %s (line %" PRI_SIZET ").\n", name, node->line);
				handle_error(compiler);
				return;
			}
			decl_var(compiler, name, child->line);
			if (!in_function(compiler) && !compiler->stack) {
				compiler_add_code_BB(compiler, O_MOVEUP_FP, (unsigned char)0);
				store_var(compiler, name, node->line);
			}
			if (child->nodetype == N_CONST) make_const(compiler, name);
		}
	}
}

static int visit_TriOp(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	struct Node *left = TriOp_get_left(node);
	struct Node *middle = TriOp_get_middle(node);
	struct Node *right = TriOp_get_right(node);

	visit_expr(compiler, left, stack_height);

	int64_t index_l;
	enter_conditional_false(compiler, &index_l);

	visit_expr(compiler, middle, stack_height);

	size_t index_r;
	enter_jump(compiler, &index_r);

	exit_conditional_false(compiler, &index_l);

	visit_expr(compiler, right, stack_height);
	exit_jump(compiler, &index_r);

	return stack_height + 1;
}

static void visit_BinOp_shortcircuit(struct Compiler *const compiler, const struct Node *const node, enum Opcode jump_type, int stack_height) {
	visit_expr(compiler, BinOp_get_left(node), stack_height);
	compiler_add_code_BBW(compiler, O_DUP, jump_type, -1);
	size_t index = compiler->buffer->count;
	compiler_add_byte(compiler, O_POP);
	visit_expr(compiler, BinOp_get_right(node), stack_height);
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, index - 8, compiler->buffer->count - index);
}

static int visit_BinOp(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	// complicated bin ops are handled on their own.
	if (node->value.type == T_DQMARK) {        // ?? operator
		visit_BinOp_shortcircuit(compiler, node, O_BRN_8, stack_height);
		return stack_height + 1;
	} else if (node->value.type == T_DBAR) {   // || operator
		visit_BinOp_shortcircuit(compiler, node, O_BRT_8, stack_height);
		return stack_height + 1;
	} else if (node->value.type == T_DAMP) {   // && operator
		visit_BinOp_shortcircuit(compiler, node, O_BRF_8, stack_height);
		return stack_height + 1;
	}

	// all other operators follow the same pattern of visiting one child then the other.
	visit_expr(compiler, BinOp_get_left(node), stack_height);
	visit_expr(compiler, BinOp_get_right(node), stack_height + 1);
	switch (node->value.type) {
	case T_BAR:
		compiler_add_byte(compiler, O_BOR);
		break;
	case T_CARET:
		compiler_add_byte(compiler, O_BXOR);
		break;
	case T_AMP:
		compiler_add_byte(compiler, O_BAND);
		break;
	case T_AMPCARET:
		compiler_add_byte(compiler, O_BANDNOT);
		break;
	case T_DEQ:
		compiler_add_byte(compiler, O_EQ);
		break;
	case T_TEQ:
		compiler_add_byte(compiler, O_ID);
		break;
	case T_BANGEQ:
		compiler_add_byte(compiler, O_EQ);
		compiler_add_byte(compiler, O_NOT);
		break;
	case T_BANGDEQ:
		compiler_add_byte(compiler, O_ID);
		compiler_add_byte(compiler, O_NOT);
		break;
	case T_GT:
		compiler_add_byte(compiler, O_GT);
		break;
	case T_GTEQ:
		compiler_add_byte(compiler, O_GE);
		break;
	case T_LT:
		compiler_add_byte(compiler, O_LT);
		break;
	case T_LTEQ:
		compiler_add_byte(compiler, O_LE);
		break;
	case T_TILDE:
		compiler_add_byte(compiler, O_CNCT);
		break;
	case T_DGT:
		compiler_add_byte(compiler, O_BSR);
		break;
	case T_DLT:
		compiler_add_byte(compiler, O_BSL);
		break;
	case T_PLUS:
		compiler_add_byte(compiler, O_ADD);
		break;
	case T_MINUS:
		compiler_add_byte(compiler, O_SUB);
		break;
	case T_STAR:
		compiler_add_byte(compiler, O_MUL);
		break;
	case T_SLASH:
		compiler_add_byte(compiler, O_FDIV);
		break;
	case T_DSLASH:
		compiler_add_byte(compiler, O_IDIV);
		break;
	case T_MOD:
		compiler_add_byte(compiler, O_MOD);
		break;
	case T_DSTAR:
		compiler_add_byte(compiler, O_EXP);
		break;
	default:
		YASL_UNREACHED();
		break;
	}

	return stack_height + 1;
}

static int visit_UnOp(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	visit_expr(compiler, UnOp_get_expr(node), stack_height);
	switch (node->value.type) {
	case T_PLUS:
		compiler_add_byte(compiler, O_POS);
		break;
	case T_MINUS:
		compiler_add_byte(compiler, O_NEG);
		break;
	case T_BANG:
		compiler_add_byte(compiler, O_NOT);
		break;
	case T_CARET:
		compiler_add_byte(compiler, O_BNOT);
		break;
	case T_LEN:
		compiler_add_byte(compiler, O_LEN);
		break;
	default:
		YASL_UNREACHED();
		break;
	}

	return stack_height + 1;
}

static void visit_Assign(struct Compiler *const compiler, const struct Node *const node) {
	char *name = node->value.sval.str;
	if (!contains_var(compiler, name)) {
		compiler_print_err_undeclared_var(compiler, name, node->line);
		handle_error(compiler);
		return;
	}
	visit_expr(compiler, Assign_get_expr(node), (int)get_stacksize(compiler));
	store_var(compiler, name, node->line);
}

static int visit_Var(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_UNUSED(stack_height);
	load_var(compiler, Var_get_name(node), node->line);
	return stack_height + 1;
}

static int visit_Undef(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_UNUSED(node);
	YASL_UNUSED(stack_height);
	compiler_add_byte(compiler, O_NCONST);
	return stack_height + 1;
}

static void compiler_add_literal(struct Compiler *const compiler, const yasl_int index) {
	if (index < 128) {
		compiler_add_code_BB(compiler, O_LIT, (unsigned char)index);
	} else {
		compiler_add_code_BW(compiler, O_LIT8, index);
	}
}

static int visit_Float(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_UNUSED(stack_height);

	yasl_float val = Float_get_float(node);

	yasl_int index = compiler_intern_float(compiler, val);
	compiler_add_literal(compiler, index);

	return stack_height + 1;
}

static int visit_Integer(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_UNUSED(stack_height);

	yasl_int val = Integer_get_int(node);
	YASL_COMPILE_DEBUG_LOG("int: %" PRId64 "\n", val);

	yasl_int index = compiler_intern_int(compiler, val);
	compiler_add_literal(compiler, index);

	return stack_height + 1;
}

static int visit_Boolean(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_UNUSED(stack_height);
	compiler_add_byte(compiler, Boolean_get_bool(node) ? O_BCONST_T : O_BCONST_F);

	return stack_height + 1;
}

static int visit_String(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_UNUSED(stack_height);

	yasl_int index = intern_string(compiler, node);
	compiler_add_literal(compiler, index);

	return stack_height + 1;
}

static void visit_Assert(struct Compiler *const compiler, const struct Node *const node) {
	visit_expr(compiler, Assert_get_expr(node), (int)get_stacksize(compiler));
	compiler_add_byte(compiler, O_ASS);
}

static void make_new_collection(struct Compiler *const compiler, const struct Node *const node, enum Opcode type, int stack_height) {
	compiler_add_byte(compiler, O_END);
	visit_expr(compiler, node, stack_height + 1);
	compiler_add_byte(compiler, type);
}

static int visit_List(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	make_new_collection(compiler, List_get_values(node), O_NEWLIST, stack_height);

	return stack_height + 1;
}

static int visit_Table(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	make_new_collection(compiler, Table_get_values(node), O_NEWTABLE, stack_height);

	return stack_height + 1;
}

/*
 * Like visit, but doesn't save the results. Will validate things like variables having been declared before use.
 */
#define DEF_VALIDATE(n, ...) static void validate_##n(struct Compiler *compiler, const struct Node *const node) {\
	const size_t buffer_count = compiler->buffer->count;\
	const size_t code_count = compiler->code->count;\
	const size_t line_count = compiler->lines->count;\
	const size_t line = compiler->line;\
	visit_##n(__VA_ARGS__);\
	compiler->buffer->count = buffer_count;\
	compiler->code->count = code_count;\
	compiler->lines->count = line_count;\
	compiler->line = line;\
}

DEF_VALIDATE(expr, compiler, node, -1)
DEF_VALIDATE(stmt, compiler, node)

static void visit_LetIter(struct Compiler *const compiler, const struct Node *const node) {
	YASL_UNUSED(compiler);
	YASL_UNUSED(node);
	YASL_UNREACHED();
}

static int visit_Vargs(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_UNUSED(stack_height);
	load_var(compiler, "...", node->line);
	compiler_add_byte(compiler, O_SPREAD_VARGS);

	return stack_height + 1;
}

static int visit_Parens(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	visit_expr(compiler, Parens_get_expr(node), stack_height);

	return stack_height + 1;
}

static int (*expr_jmp_table[])(struct Compiler *const compiler, const struct Node *const node, int stack_height) = {
#define X(name, ...) &visit_##name,
#include "exprnodetype.x"
#undef X
#define X(...) NULL,
#include "pattnodetype.x"
#include "stmtnodetype.x"
#undef X
};

static void (*jmp_table[])(struct Compiler *const compiler, const struct Node *const node) = {
#define X(...) NULL,
#include "exprnodetype.x"
#undef X
#define X(name, ...) &visit_##name,
#include "pattnodetype.x"
#include "stmtnodetype.x"
#undef X
};

static void setline(struct Compiler *const compiler, const struct Node *const node) {
	while (node->line > compiler->line) {
		YASL_ByteBuffer_add_vint(compiler->lines, compiler->code->count + compiler->buffer->count);
		compiler->line++;
	}
}

static void visit(struct Compiler *const compiler, const struct Node *const node) {
	setline(compiler, node);

	jmp_table[node->nodetype](compiler, node);
}

#ifdef YASL_DEBUG
const char* node_name(const struct Node *const node) {
	switch (node->nodetype) {
#define X(name, e, ...) case e: return #name;
#include "nodetype.x"
#undef X
	default:
		YASL_UNREACHED();
		return NULL;
	}
}

static bool is_expr(const struct Node *const node) {
	switch (node->nodetype) {
#define X(name, e, ...) case e: return true;
#include "exprnodetype.x"
#undef X
	default:
		return false;
	}
}

static bool is_patt(const struct Node *const node) {
	switch (node->nodetype) {
#define X(name, e, ...) case e: return true;
#include "pattnodetype.x"
#undef X
	default:
		return false;
	}
}

static bool is_stmt(const struct Node *const node) {
	switch (node->nodetype) {
#define X(name, e, ...) case e: return true;
#include "stmtnodetype.x"
#undef X
	default:
		return false;
	}
}
#endif

static int visit_expr(struct Compiler *const compiler, const struct Node *const node, int stack_height) {
	YASL_ASSERT(is_expr(node), "Expected expression");
	setline(compiler, node);


	YASL_ASSERT(stack_height >= 0, "expected non-negative stack height");
/*
#ifdef YASL_DEBUG
	compiler_add_byte(compiler, O_ASSERT_STACK_HEIGHT);
	compiler_add_byte(compiler, stack_height);
#endif  // YASL_DEBUG
*/
	return expr_jmp_table[node->nodetype](compiler, node, stack_height);
}

static void visit_patt(struct Compiler *const compiler, const struct Node *const node) {
	// printf("Node: %s\n", node_name(node));
	YASL_ASSERT(is_patt(node), "Expected pattern");
	visit(compiler, node);
}

static void visit_stmt(struct Compiler *const compiler, const struct Node *const node) {
	YASL_ASSERT(is_stmt(node), "Expected statement");
	visit(compiler, node);
}
