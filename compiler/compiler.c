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

static void validate(struct Compiler *compiler, const struct Node *const node);

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
	free(compiler->locals);
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

static void enter_scope(struct Compiler *const compiler) {
	struct Scope **lval = in_function(compiler) ? &compiler->params->scope : &compiler->stack;
	*lval = scope_new(*lval);
}

static void exit_scope(struct Compiler *const compiler) {
	struct Scope **lval = in_function(compiler) ? &compiler->params->scope : &compiler->stack;
	struct Scope *tmp = *lval;
	 *lval = tmp->parent;
	size_t num_locals = tmp->vars.count;
	scope_del_current_only(tmp);
	while (num_locals-- > 0) {
		compiler_add_byte(compiler, O_POP);
	}
}

static inline void enter_conditional_false(struct Compiler *const compiler, int64_t *const index) {
	compiler_add_byte(compiler, O_BRF_8);
	*index = compiler->buffer->count;
	compiler_add_int(compiler, 0);
}

static inline void exit_conditional_false(const struct Compiler *const compiler, const int64_t *const index) {
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, (size_t) *index, compiler->buffer->count - *index - 8);
}

static void sizebuffer_push(struct SizeBuffer *sb, const size_t item) {
	if (sb->count >= sb->size) {
		sb->size *= 2;
		sb->items = (size_t *)realloc(sb->items, sizeof(size_t) * sb->size);
	}
	sb->items[sb->count++] = item;
}

static void sizebuffer_pop(struct SizeBuffer *sb) {
	sb->count--;
}

static void add_checkpoint(struct Compiler *const compiler, const size_t cp) {
	sizebuffer_push(&compiler->checkpoints, cp);
}

static void rm_checkpoint(struct Compiler *const compiler) {
	sizebuffer_pop(&compiler->checkpoints);
}

static void visit(struct Compiler *const compiler, const struct Node *const node);

static void visit_Body(struct Compiler *const compiler, const struct Node *const node) {
	FOR_CHILDREN(i, child, node) {
		visit(compiler, child);
	}
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

static struct Env *get_nearest(struct Env *env, const char *const name) {
	while (!env_contains_cur_only(env, name)) {
		env = env->parent;
	}
	return env;
}

static void load_var_local(struct Compiler *const compiler, const struct Scope *scope, const char *const name) {
	int64_t index = get_index(scope_get(scope, name));
	compiler_add_byte(compiler, O_LLOAD);
	compiler_add_byte(compiler, (unsigned char) index);
}

static void load_var_from_upval(struct Compiler *const compiler, const char *const name) {
	compiler->params->isclosure = true;
	compiler_add_byte(compiler, O_ULOAD);
	yasl_int tmp = env_resolve_upval_index(compiler->params, compiler->stack, name);
	compiler_add_byte(compiler, (unsigned char) tmp);
}

static void load_var(struct Compiler *const compiler, const char *const name, const size_t line) {
	const size_t name_len = strlen(name);
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
		compiler_add_byte(compiler, O_GLOAD_8);
		compiler_add_int(compiler, YASL_Table_search_string_int(compiler->strings, name, name_len).value.ival);
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
	compiler_add_byte(compiler, O_LSTORE);
	compiler_add_byte(compiler, (unsigned char) index);
}

static void store_var_in_upval(struct Compiler *const compiler, const char *const name) {
	compiler->params->isclosure = true;
	yasl_int index = env_resolve_upval_index(compiler->params, compiler->stack, name);
	compiler_add_byte(compiler, O_USTORE);
	compiler_add_byte(compiler, (unsigned char) index);
}

static void store_var(struct Compiler *const compiler, const char *const name, const size_t line) {
	const size_t name_len = strlen(name);
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
		compiler_add_byte(compiler, O_GSTORE_8);
		compiler_add_int(compiler, YASL_Table_search_string_int(compiler->strings, name, name_len).value.ival);
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
			YASL_BYTECODE_DEBUG_LOG("%02x\n", compiler->header->bytes[i]);
		else
			YASL_BYTECODE_DEBUG_LOG("%02x ", compiler->header->bytes[i]);
	}
	YASL_BYTECODE_DEBUG_LOG("\n%s\n", "entry point");
	for (size_t i = 0; i < compiler->code->count; i++) {
		if (i % 16 == 15)
			YASL_BYTECODE_DEBUG_LOG("%02x\n", compiler->code->bytes[i]);
		else
			YASL_BYTECODE_DEBUG_LOG("%02x ", compiler->code->bytes[i]);
	}
	YASL_BYTECODE_DEBUG_LOG("%02x\n", O_HALT);
	YASL_BYTECODE_DEBUG_LOG("%s\n", "lines");
	for (size_t i = 0; i < compiler->lines->count; i++) {
		if (i % 16 == 15)
			YASL_BYTECODE_DEBUG_LOG("%02x\n", compiler->lines->bytes[i]);
		else
			YASL_BYTECODE_DEBUG_LOG("%02x ", compiler->lines->bytes[i]);
	}
	YASL_BYTECODE_DEBUG_LOG("%s", "\n");

	fflush(stdout);
	unsigned char *bytecode = (unsigned char *) malloc(
		compiler->code->count + compiler->header->count + 1 + compiler->lines->count);    // NOT OWN
	memcpy(bytecode, compiler->header->bytes, compiler->header->count);
	memcpy(bytecode + compiler->header->count, compiler->code->bytes, compiler->code->count);
	bytecode[compiler->code->count + compiler->header->count] = O_HALT;
	memcpy(bytecode + compiler->code->count + 1 + compiler->header->count, compiler->lines->bytes, compiler->lines->count);
	return bytecode;
}

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
		visit(compiler, node);
		YASL_ByteBuffer_extend(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
		compiler->buffer->count = 0;

		node_del(node);
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
			visit(compiler, node);
			YASL_ByteBuffer_extend(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
			compiler->buffer->count = 0;
		}

		node_del(node);
	}

	return return_bytes(compiler);
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
		validate(compiler, expr);
		return;
	default:
		visit(compiler, expr);
		if (expr->nodetype == N_ASSIGN || expr->nodetype == N_SET) {
			return;
		} else {
			compiler_add_byte(compiler, O_POP);
		}
	}
}

static void visit_FnDecl(struct Compiler *const compiler, const struct Node *const node) {
	compiler->params = env_new(compiler->params);

	enter_scope(compiler);

	FOR_CHILDREN(i, child, FnDecl_get_params(node)) {
		decl_var(compiler, Decl_get_name(child), child->line);
		if (child->nodetype == N_CONST) {
			make_const(compiler, Decl_get_name(child));
		}
	}

	compiler_add_byte(compiler, O_FCONST);
	compiler_add_int(compiler, 0);

	size_t old_size = compiler->buffer->count;

	// TODO: verfiy that number of params is small enough. (same for the other casts below.)
	compiler_add_byte(compiler, (unsigned char) Body_get_len(FnDecl_get_params(node)));
	visit_Body(compiler, FnDecl_get_body(node));
	// TODO: remove this when it's not required.
	compiler_add_byte(compiler, O_NCONST);
	compiler_add_byte(compiler, compiler->params->usedinclosure ? O_CRET : O_RET);
	compiler_add_byte(compiler, (unsigned char)1);
	exit_scope(compiler);

	size_t new_size = compiler->buffer->count;
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, old_size - sizeof(yasl_int), new_size - old_size);

	if (compiler->params->isclosure) {
		compiler->buffer->bytes[old_size - sizeof(yasl_int) - 1] = O_CCONST;
		const size_t count = compiler->params->upval_indices.count;
		compiler_add_byte(compiler, (unsigned char) count);
		const size_t start = compiler->buffer->count;
		// TODO what's below is wrong. We need to get the right value for the upvals.
		for (size_t i = 0; i < count; i++) {
			compiler_add_byte(compiler, 0);
		}
		FOR_TABLE(i, item, &compiler->params->upval_indices) {
			int64_t index = item->value.value.ival;
			int64_t value = YASL_Table_search(&compiler->params->upval_values, item->key).value.ival;
			compiler->buffer->bytes[start + index] = value;
		}
	}

	struct Env *tmp = compiler->params->parent;
	compiler->params->parent = NULL;
	env_del(compiler->params);
	compiler->params = tmp;
}

static void visit_VariadicContext(struct Compiler *const compiler, const struct Node *const node) {
	int old_returns = compiler->expected_returns;
	compiler->expected_returns = node->value.ival;
	visit(compiler, node->children[0]);
	compiler->expected_returns = old_returns;
}

static void visit_Call(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Call_get_object(node));
	compiler_add_byte(compiler, O_INIT_CALL);
	compiler_add_byte(compiler, (unsigned char)compiler->expected_returns);
	visit_Body(compiler, Call_get_params(node));
	compiler_add_byte(compiler, O_CALL);
}

static void visit_MethodCall(struct Compiler *const compiler, const struct Node *const node) {
	char *str = MethodCall_get_name(node);
	size_t len = strlen(str);
	visit(compiler, MethodCall_get_object(node));

	yasl_int index = compiler_intern_string(compiler, str, len);

	compiler_add_byte(compiler, O_INIT_MC);
	compiler_add_byte(compiler, (unsigned char)compiler->expected_returns);
	compiler_add_int(compiler, index);

	visit_Body(compiler, MethodCall_get_params(node));
	compiler_add_byte(compiler, O_CALL);
}

static void visit_Return(struct Compiler *const compiler, const struct Node *const node) {
	if (!in_function(compiler)) {
		compiler_print_err_syntax(compiler, "`return` outside of function (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}
	visit(compiler, Return_get_expr(node));
	compiler_add_byte(compiler, compiler->params->usedinclosure ? O_CRET : O_RET);
	compiler_add_byte(compiler, (unsigned char)1);
}

static void visit_MultiReturn(struct Compiler *const compiler, const struct Node *const node) {
	if (!in_function(compiler)) {
		compiler_print_err_syntax(compiler, "`return` outside of function (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}

	visit(compiler, MultiReturn_get_exprs(node));
	compiler_add_byte(compiler, compiler->params->usedinclosure ? O_CRET : O_RET);
	compiler_add_byte(compiler, (unsigned char)MultiReturn_get_exprs(node)->children_len);
}

static void visit_Export(struct Compiler *const compiler, const struct Node *const node) {
	if (in_function(compiler) || compiler->stack && compiler->stack->parent) {
		compiler_print_err_syntax(compiler, "`export` statement must be at top level of module (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}
	visit(compiler, Export_get_expr(node));
	compiler_add_byte(compiler, O_EXPORT);
}

static void visit_Set(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Set_get_collection(node));
	visit(compiler, Set_get_key(node));
	visit(compiler, Set_get_value(node));
	compiler_add_byte(compiler, O_SET);
}

static void visit_Get(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Get_get_collection(node));
	visit(compiler, Get_get_value(node));
	compiler_add_byte(compiler, O_GET);
}

static void visit_Slice(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Slice_get_collection(node));
	visit(compiler, Slice_get_start(node));
	visit(compiler, Slice_get_end(node));
	compiler_add_byte(compiler, O_SLICE);
}

static void visit_Block(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);
	visit(compiler, Block_get_block(node));
	exit_scope(compiler);
}

static inline void branch_back(struct Compiler *const compiler, int64_t index) {
	compiler_add_byte(compiler, O_BR_8);
	compiler_add_int(compiler, index - compiler->buffer->count - 8);
}

static void visit_Comp_cond(struct Compiler *const compiler, const struct Node *const cond, const struct Node *const expr) {
	if (cond) {
		int64_t index_third;
		visit(compiler, cond);
		enter_conditional_false(compiler, &index_third);

		visit(compiler, expr);

		exit_conditional_false(compiler, &index_third);
	} else {
		visit(compiler, expr);
	}
}

static void visit_Comp(struct Compiler *const compiler, const struct Node *const node, unsigned char byte) {
	enter_scope(compiler);

	struct Node *expr = Comp_get_expr(node);
	struct Node *cond = Comp_get_cond(node);
	struct Node *iter = Comp_get_iter(node);

	struct Node *collection = LetIter_get_collection(iter);

	char *name = iter->value.sval.str;

	visit(compiler, collection);

	compiler_add_byte(compiler, O_INITFOR);
	compiler_add_byte(compiler, O_END);
	decl_var(compiler, name, iter->line);
	compiler_add_byte(compiler, O_END);

	int64_t index_start = compiler->buffer->count;

	compiler_add_byte(compiler, O_ITER_1);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, name, iter->line);

	visit_Comp_cond(compiler, cond, expr);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	compiler_add_byte(compiler, byte);
	compiler_add_byte(compiler, O_ENDCOMP);

	size_t curr = compiler->buffer->count;
	exit_scope(compiler);
	compiler->buffer->count = curr;
}

static void visit_ListComp(struct Compiler *const compiler, const struct Node *const node) {
	visit_Comp(compiler, node, O_NEWLIST);
}

static void visit_TableComp(struct Compiler *const compiler, const struct Node *const node) {
	visit_Comp(compiler, node, O_NEWTABLE);
}

static void visit_ForIter(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);

	struct Node *iter = ForIter_get_iter(node);
	struct Node *body = ForIter_get_body(node);

	struct Node *collection = LetIter_get_collection(iter);
	char *name = iter->value.sval.str;

	visit(compiler, collection);

	compiler_add_byte(compiler, O_INITFOR);
	compiler_add_byte(compiler, O_END);
	decl_var(compiler, name, iter->line);

	size_t index_start = compiler->buffer->count;
	add_checkpoint(compiler, index_start);

	compiler_add_byte(compiler, O_ITER_1);

	add_checkpoint(compiler, compiler->buffer->count);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, name, iter->line);

	visit(compiler, body);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	compiler_add_byte(compiler, O_ENDFOR);
	exit_scope(compiler);

	rm_checkpoint(compiler);
	rm_checkpoint(compiler);
}

static void enter_jump(struct Compiler *const compiler, size_t *index) {
	compiler_add_byte(compiler, O_BR_8);
	*index = compiler->buffer->count;
	compiler_add_int(compiler, 0);
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
	validate(compiler, body);
	if (post) validate(compiler, post);
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
		visit(compiler, post);
		exit_jump(compiler, &index);
	}

	add_checkpoint(compiler, index_start);

	visit(compiler, cond);

	add_checkpoint(compiler, compiler->buffer->count);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	visit(compiler, body);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

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
	(void) node;
	compiler_add_byte(compiler, P_UNDEF);
}

static void visit_BoolPattern(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, P_BOOL);
	compiler_add_byte(compiler, (unsigned char)((bool)node->value.ival ? 1 : 0));
}

static void compiler_add_literal_pattern(struct Compiler *const compiler, const yasl_int index) {
	if (index < 128) {
		compiler_add_byte(compiler, P_LIT);
		compiler_add_byte(compiler, (unsigned char)index);
	} else {
		compiler_add_byte(compiler, P_LIT8);
		compiler_add_int(compiler, index);
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
	compiler_add_byte(compiler, byte);
	compiler_add_int(compiler, node->children_len);
	bool old = compiler->leftmost_pattern;
	FOR_CHILDREN(i, child, node) {
			visit(compiler, child);
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
	YASL_Table_insert_string_int(&compiler->seen_bindings, name, strlen(name), 1);
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
	(void) node;
	compiler_add_byte(compiler, P_ANY);
}

static void visit_AltPattern(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, P_ALT);
	struct YASL_Table prev = compiler->seen_bindings;
	compiler->seen_bindings = NEW_TABLE();
	visit(compiler, BinOp_get_left(node));

	if (compiler->status) {
		DEL_TABLE(&prev);
		return;
	}

	struct YASL_Table old = compiler->seen_bindings;
	(void) old;
	compiler->seen_bindings = NEW_TABLE();
	compiler->leftmost_pattern = false;

	visit(compiler, BinOp_get_right(node));

	if (compiler->status) {
		goto cleanup;
	}

	FOR_TABLE(i, item, &old) {
		struct YASL_Object val = YASL_Table_search(&compiler->seen_bindings, item->key);
		if (val.type == Y_END) {
			compiler_print_err_syntax(compiler, "%.*s not bound on right side of | (line %" PRI_SIZET ").\n", (int)YASL_String_len(item->key.value.sval), item->key.value.sval->str, node->line);
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

static void visit_Match_helper(struct Compiler *const compiler, const struct Node *const patterns, const struct Node *const guards, const struct Node *const bodies, size_t curr) {
	compiler_add_byte(compiler, O_MATCH);
	size_t start = compiler->buffer->count;
	compiler_add_int(compiler, 0);

	size_t vars = get_scope_in_use(compiler)->vars.count;
	enter_scope(compiler);
	compiler->leftmost_pattern = true;
	visit(compiler, patterns->children[curr]);

	struct Node *guard = guards->children[curr];
	size_t start_guard = 0;

	unsigned char bindings = (unsigned char) get_scope_in_use(compiler)->vars.count;
	if (bindings) {
		compiler_add_byte(compiler, O_INCSP);
		compiler_add_byte(compiler, bindings);
		if (guard) {
			compiler_add_byte(compiler, O_MOVEUP_FP);
			compiler_add_byte(compiler, (unsigned char) vars);
			visit(compiler, guard);
			compiler_add_byte(compiler, O_BRF_8);
			start_guard = compiler->buffer->count;
			compiler_add_int(compiler, 0);
			compiler_add_byte(compiler, O_POP);
		} else {
			compiler_add_byte(compiler, O_DEL_FP);
			compiler_add_byte(compiler, (unsigned char) vars);
		}
	} else {
		if (guard) {
			visit(compiler, guard);
			compiler_add_byte(compiler, O_BRF_8);
			start_guard = compiler->buffer->count;
			compiler_add_int(compiler, 0);
		}
		compiler_add_byte(compiler, O_POP);
	}

	visit(compiler, bodies->children[curr]);
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
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, start, compiler->buffer->count - start - 8);

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
	visit(compiler, expr);
	if (patterns->children_len == 0) {
		compiler_add_byte(compiler, O_POP);
		return;
	}

	visit_Match_helper(compiler, patterns, guards, bodies, 0);
}

static void visit_If_true(struct Compiler *const compiler, const struct Node *const then_br, const struct Node *const else_br) {
	visit(compiler, then_br);
	if (else_br) validate(compiler, else_br);
}

static void visit_If_false(struct Compiler *const compiler, const struct Node *const then_br, const struct Node *const else_br) {
	validate(compiler, then_br);
	if (else_br) visit(compiler, else_br);
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

	visit(compiler, cond);

	int64_t index_then;
	enter_conditional_false(compiler, &index_then);
	visit(compiler, then_br);

	size_t index_else = 0;

	if (else_br) {
		enter_jump(compiler, &index_else);
	}


	exit_conditional_false(compiler, &index_then);

	if (else_br) {
		visit(compiler, else_br);
		exit_jump(compiler, &index_else);
	}
}

static void visit_Echo(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Echo_get_expr(node));
	compiler_add_byte(compiler, O_ECHO);
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
		visit(compiler, expr);
	} else {
		if (expr) visit(compiler, expr);
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
	visit(compiler, Decl_get_rvals(node));

	FOR_CHILDREN(i, child, Decl_get_lvals(node)) {
		const char *name = child->value.sval.str;
		if (child->nodetype == N_ASSIGN) {
			if (!contains_var(compiler, name)) {
				compiler_print_err_undeclared_var(compiler, name, node->line);
				handle_error(compiler);
				return;
			}
			compiler_add_byte(compiler, O_MOVEUP_FP);
			compiler_add_byte(compiler, (unsigned char)(get_scope_in_use(compiler)->vars.count));
			store_var(compiler, name, node->line);
		} else if (child->nodetype == N_SET) {
			visit(compiler, Set_get_collection(child));
			visit(compiler, Set_get_key(child));
			compiler_add_byte(compiler, O_MOVEUP_FP);
			compiler_add_byte(compiler, (unsigned char)(get_scope_in_use(compiler)->vars.count));
			compiler_add_byte(compiler, O_SET);
		} else {
			if (contains_var_in_current_scope(compiler, name)) {
				compiler_print_err_syntax(compiler, "Illegal redeclaration of %s (line %" PRI_SIZET ").\n", name, node->line);
				handle_error(compiler);
				return;
			}
			decl_var(compiler, name, child->line);
			if (!in_function(compiler) && !compiler->stack) {
				compiler_add_byte(compiler, O_MOVEUP_FP);
				compiler_add_byte(compiler, (unsigned char)0);
				store_var(compiler, name, node->line);
			}
			if (child->nodetype == N_CONST) make_const(compiler, name);
		}
	}
}

static void visit_TriOp(struct Compiler *const compiler, const struct Node *const node) {
	struct Node *left = TriOp_get_left(node);
	struct Node *middle = TriOp_get_middle(node);
	struct Node *right = TriOp_get_right(node);

	visit(compiler, left);

	int64_t index_l;
	enter_conditional_false(compiler, &index_l);

	visit(compiler, middle);

	size_t index_r;
	enter_jump(compiler, &index_r);

	exit_conditional_false(compiler, &index_l);

	visit(compiler, right);
	exit_jump(compiler, &index_r);
}

static void visit_BinOp_shortcircuit(struct Compiler *const compiler, const struct Node *const node, enum Opcode jump_type) {
	visit(compiler, BinOp_get_left(node));
	compiler_add_byte(compiler, O_DUP);
	compiler_add_byte(compiler, jump_type);
	size_t index = compiler->buffer->count;
	compiler_add_int(compiler, 0);
	compiler_add_byte(compiler, O_POP);
	visit(compiler, BinOp_get_right(node));
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, index, compiler->buffer->count - index - 8);
}

static void visit_BinOp(struct Compiler *const compiler, const struct Node *const node) {
	// complicated bin ops are handled on their own.
	if (node->value.type == T_DQMARK) {        // ?? operator
		visit_BinOp_shortcircuit(compiler, node, O_BRN_8);
		return;
	} else if (node->value.type == T_DBAR) {   // || operator
		visit_BinOp_shortcircuit(compiler, node, O_BRT_8);
		return;
	} else if (node->value.type == T_DAMP) {   // && operator
		visit_BinOp_shortcircuit(compiler, node, O_BRF_8);
		return;
	}

	// all other operators follow the same pattern of visiting one child then the other.
	visit(compiler, BinOp_get_left(node));
	visit(compiler, BinOp_get_right(node));
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
}

static void visit_UnOp(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, UnOp_get_expr(node));
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
}

static void visit_Assign(struct Compiler *const compiler, const struct Node *const node) {
	char *name = node->value.sval.str;
	if (!contains_var(compiler, name)) {
		compiler_print_err_undeclared_var(compiler, name, node->line);
		handle_error(compiler);
		return;
	}
	visit(compiler, Assign_get_expr(node));
	store_var(compiler, name, node->line);
}

static void visit_Var(struct Compiler *const compiler, const struct Node *const node) {
	load_var(compiler, Var_get_name(node), node->line);
}

static void visit_Undef(struct Compiler *const compiler, const struct Node *const node) {
	(void) node;
	compiler_add_byte(compiler, O_NCONST);
}

static void compiler_add_literal(struct Compiler *const compiler, const yasl_int index) {
	if (index < 128) {
		compiler_add_byte(compiler, O_LIT);
		compiler_add_byte(compiler, (unsigned char)index);
	} else {
		compiler_add_byte(compiler, O_LIT8);
		compiler_add_int(compiler, index);
	}
}

static void visit_Float(struct Compiler *const compiler, const struct Node *const node) {
	yasl_float val = Float_get_float(node);

	yasl_int index = compiler_intern_float(compiler, val);
	compiler_add_literal(compiler, index);
}

static void visit_Integer(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int val = Integer_get_int(node);
	YASL_COMPILE_DEBUG_LOG("int: %" PRId64 "\n", val);

	yasl_int index = compiler_intern_int(compiler, val);
	compiler_add_literal(compiler, index);
}

static void visit_Boolean(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, Boolean_get_bool(node) ? O_BCONST_T : O_BCONST_F);
}

static void visit_String(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int index = intern_string(compiler, node);
	compiler_add_literal(compiler, index);
}

static void visit_Assert(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Assert_get_expr(node));
	compiler_add_byte(compiler, O_ASS);
}

static void make_new_collection(struct Compiler *const compiler, const struct Node *const node, enum Opcode type) {
	compiler_add_byte(compiler, O_END);
	visit_Body(compiler, node);
	compiler_add_byte(compiler, type);
}

static void visit_List(struct Compiler *const compiler, const struct Node *const node) {
	make_new_collection(compiler, List_get_values(node), O_NEWLIST);
}

static void visit_Table(struct Compiler *const compiler, const struct Node *const node) {
	make_new_collection(compiler, Table_get_values(node), O_NEWTABLE);
}

/*
 * Like visit, but doesn't save the results. Will validate things like variables having been declared before use.
 */
static void validate(struct Compiler *compiler, const struct Node *const node) {
	const size_t buffer_count = compiler->buffer->count;
	const size_t code_count = compiler->code->count;
	const size_t line_count = compiler->lines->count;
	const size_t line = compiler->line;
	visit(compiler, node);
	compiler->buffer->count = buffer_count;
	compiler->code->count = code_count;
	compiler->lines->count = line_count;
	compiler->line = line;
}

static void visit_LetIter(struct Compiler *const compiler, const struct Node *const node) {
	(void) compiler;
	(void) node;
	YASL_UNREACHED();
}

#define X(name, ...) &visit_##name,
static void (*jmp_table[])(struct Compiler *const compiler, const struct Node *const node) = {
#include "nodetype.x"
};
#undef X

static void visit(struct Compiler *const compiler, const struct Node *const node) {
	while (node->line > compiler->line) {
		YASL_ByteBuffer_add_vint(compiler->lines, compiler->code->count + compiler->buffer->count);
		compiler->line++;
	}

	jmp_table[node->nodetype](compiler, node);
}
