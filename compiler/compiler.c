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
	if (in_function(compiler)) {
		compiler->params->scope = scope_new(compiler->params->scope);
	} else {
		compiler->stack = scope_new(compiler->stack);
	}
}

static void exit_scope(struct Compiler *const compiler) {
	if (in_function(compiler)) {
		size_t num_locals = compiler->params->scope->vars.count;
		struct Scope *tmp = compiler->params->scope;
		compiler->params->scope = compiler->params->scope->parent;
		scope_del_current_only(tmp);
		while (num_locals-- > 0) {
			compiler_add_byte(compiler, O_POP);
		}
	} else {
		size_t num_locals = compiler->stack->vars.count;
		struct Scope *tmp = compiler->stack;
		compiler->stack = compiler->stack->parent;
		scope_del_current_only(tmp);
		while (num_locals-- > 0) {
			compiler_add_byte(compiler, O_POP);
		}
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

static void load_var(struct Compiler *const compiler, const char *const name, const size_t line) {
	const size_t name_len = strlen(name);
	if (compiler->params && scope_contains(compiler->params->scope, name)) {   // function-local var
		int64_t index = get_index(scope_get(compiler->params->scope, name));
		compiler_add_byte(compiler, O_LLOAD);
		compiler_add_byte(compiler, (unsigned char) index);
	} else if (env_contains(compiler->params, name)) {                         // closure over function-local variable
		compiler->params->isclosure = true;
		struct Env *curr = compiler->params;
		while (!env_contains_cur_only(curr, name)) {
			curr = curr->parent;
		}
		curr->usedinclosure = true;
		compiler_add_byte(compiler, O_ULOAD);
		yasl_int tmp = env_resolve_upval_index(compiler->params, compiler->stack, name);
		compiler_add_byte(compiler, (unsigned char) tmp);
	} else if (compiler->params && scope_contains(compiler->stack, name)) {    // closure over file-local var
		compiler->params->isclosure = true;
		compiler_add_byte(compiler, O_ULOAD);
		yasl_int tmp = env_resolve_upval_index(compiler->params, compiler->stack, name);
		compiler_add_byte(compiler, (unsigned char) tmp);
	} else if (scope_contains(compiler->stack, name)) {                        // file-local vars
		int64_t index = get_index(scope_get(compiler->stack, name));
		compiler_add_byte(compiler, O_LLOAD);
		compiler_add_byte(compiler, (unsigned char) index);
	} else if (scope_contains(compiler->globals, name)) {                      // global vars
		compiler_add_byte(compiler, O_GLOAD_8);
		compiler_add_int(compiler,
					YASL_Table_search_string_int(compiler->strings, name, name_len).value.ival);
	} else {
		compiler_print_err_undeclared_var(compiler, name, line);
		handle_error(compiler);
		return;
	}
}

static void store_var(struct Compiler *const compiler, const char *const name, const size_t line) {
	const size_t name_len = strlen(name);
	if (compiler->params && scope_contains(compiler->params->scope, name)) {  // function-local variable
		int64_t index = scope_get(compiler->params->scope, name);
		if (is_const(index)) {
			compiler_print_err_const(compiler, name, line);
			handle_error(compiler);
			return;
		}
		compiler_add_byte(compiler, O_LSTORE);
		compiler_add_byte(compiler, (unsigned char) index);
	} else if (env_contains(compiler->params, name)) {                        // closure over function-local variable
		compiler->params->isclosure = true;
		struct Env *curr = compiler->params;
		while (!env_contains_cur_only(curr, name)) {
			curr = curr->parent;
		}
		curr->usedinclosure = true;
		int64_t index = scope_get(curr->scope, name);
		if (is_const(index)) {
			compiler_print_err_const(compiler, name, line);
			handle_error(compiler);
			return;
		}
		compiler_add_byte(compiler, O_USTORE);
		yasl_int tmp = env_resolve_upval_index(compiler->params, compiler->stack, name);
		compiler_add_byte(compiler, (unsigned char) tmp);
	} else if (compiler->params && scope_contains(compiler->stack, name)) {   // closure over file-local var
		int64_t index = scope_get(compiler->stack, name);
		if (is_const(index)) {
			compiler_print_err_const(compiler, name, line);
			handle_error(compiler);
			return;
		}
		compiler->params->isclosure = true;
		compiler_add_byte(compiler, O_USTORE);
		yasl_int tmp = env_resolve_upval_index(compiler->params, compiler->stack, name);
		compiler_add_byte(compiler, (unsigned char) tmp);
	} else if (scope_contains(compiler->stack, name)) {                       // file-local vars
		int64_t index = scope_get(compiler->stack, name);
		if (is_const(index)) {
			compiler_print_err_const(compiler, name, line);
			handle_error(compiler);
			return;
		}
		compiler_add_byte(compiler, O_LSTORE);
		compiler_add_byte(compiler, (unsigned char) index);
	} else if (scope_contains(compiler->globals, name)) {                     // global vars
		int64_t index = scope_get(compiler->globals, name);
		if (is_const(index)) {
			compiler_print_err_const(compiler, name, line);
			handle_error(compiler);
			return;
		}
		compiler_add_byte(compiler, O_GSTORE_8);
		compiler_add_int(compiler,
					YASL_Table_search_string_int(compiler->strings, name, name_len).value.ival);
	} else {
		compiler_print_err_undeclared_var(compiler, name, line);
		handle_error(compiler);
		return;
	}
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
	if (in_function(compiler)) {
		int64_t index = scope_decl_var(compiler->params->scope, name);
		if (index > 255) {
			compiler_print_err_syntax(compiler, "Too many variables in current scope (line %" PRI_SIZET ").\n",  line);
			handle_error(compiler);
		}
	} else if (compiler->stack) {
		int64_t index = scope_decl_var(compiler->stack, name);
		if (index > 255) {
			compiler_print_err_syntax(compiler, "Too many variables in current scope (line %" PRI_SIZET ").\n",  line);
			handle_error(compiler);
		}
	} else {
		struct YASL_Object value = YASL_Table_search_string_int(compiler->strings, name, name_len);
		if (value.type == Y_END) {
			YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
			YASL_Table_insert_string_int(compiler->strings, name, name_len, compiler->strings->count);
			YASL_ByteBuffer_add_int(compiler->header, name_len);
			YASL_ByteBuffer_extend(compiler->header, (unsigned char *) name, name_len);
		}
		scope_decl_var(compiler->globals, name);
	}
}

static void make_const(const struct Compiler * const compiler, const char *const name) {
	if (in_function(compiler)) scope_make_const(compiler->params->scope, name);
	else if (compiler->stack) scope_make_const(compiler->stack, name);
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
				node->nodetype = N_PRINT;
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

static void visit_FunctionDecl(struct Compiler *const compiler, const struct Node *const node) {
	// start logic for function.
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
	compiler_add_byte(compiler, 0);  // TODO: remove this
	visit_Body(compiler, FnDecl_get_body(node));
	// TODO: remove this when it's not required.
	compiler_add_byte(compiler, O_NCONST);
	compiler_add_byte(compiler, compiler->params->usedinclosure ? O_CRET : O_RET);
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

static void visit_Call(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Call_get_object(node));
	compiler_add_byte(compiler, O_INIT_CALL);
	visit_Body(compiler, Call_get_params(node));
	compiler_add_byte(compiler, O_CALL);
}

static void visit_MethodCall(struct Compiler *const compiler, const struct Node *const node) {
	char *str = MCall_get_name(node);
	size_t len = strlen(str);
	visit(compiler, Call_get_object(node));

	struct YASL_Object value = YASL_Table_search_string_int(compiler->strings, str, len);
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
		YASL_Table_insert_string_int(compiler->strings, str, len, compiler->strings->count);
		YASL_ByteBuffer_add_int(compiler->header, len);
		YASL_ByteBuffer_extend(compiler->header, (unsigned char *) str, len);
	}

	value = YASL_Table_search_string_int(compiler->strings, str, len);

	compiler_add_byte(compiler, O_INIT_MC);
	compiler_add_int(compiler, value.value.ival);

	visit_Body(compiler, Call_get_params(node));
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
}

static void visit_Export(struct Compiler *const compiler, const struct Node *const node) {
	if (compiler->params || compiler->stack && compiler->stack->parent) {
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

	struct Node *expr = ListComp_get_expr(node);
	struct Node *cond = ListComp_get_cond(node);
	struct Node *iter = ListComp_get_iter(node);

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

static yasl_int intern_string(struct Compiler *const compiler, const struct Node *const node) {
	const char *const str = String_get_str(node);
	size_t len = String_get_len(node);

	struct YASL_Object value = YASL_Table_search_string_int(compiler->strings, str, len);
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
		YASL_Table_insert_string_int(compiler->strings, str, len, compiler->strings->count);
		YASL_ByteBuffer_add_int(compiler->header, len);
		YASL_ByteBuffer_extend(compiler->header, (unsigned char *) str, len);
	}

	value = YASL_Table_search_string_int(compiler->strings, str, len);

	return value.value.ival;
}

static void visit_UndefPattern(struct Compiler *const compiler, const struct Node *const node) {
	(void) node;
	compiler_add_byte(compiler, P_UNDEF);
}

static void visit_BoolPattern(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, P_BOOL);
	compiler_add_byte(compiler, (unsigned char)((bool)node->value.ival ? 1 : 0));
}

static void visit_FloatPattern(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, P_FL);
	YASL_ByteBuffer_add_float(compiler->buffer, node->value.dval);
}

static void visit_IntPattern(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, P_INT);
	compiler_add_int(compiler, node->value.ival);
}

static void visit_StringPattern(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int index = intern_string(compiler, node);
	compiler_add_byte(compiler, P_STR);
	compiler_add_int(compiler, index);
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

static void visit_VarTablePattern(struct Compiler *const compiler, const struct Node *const node) {
	visit_CollectionPattern(compiler, node, P_VTABLE);
}

static void visit_VarListPattern(struct Compiler *const compiler, const struct Node *const node) {
	visit_CollectionPattern(compiler, node, P_VLS);
}

static struct Scope *get_scope_in_use(struct Compiler *const compiler) {
	return in_function(compiler) ? compiler->params->scope : compiler->stack;
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

static void visit_Match_helper(struct Compiler *const compiler, const struct Node *const patterns, const struct Node *const bodies, size_t curr) {
	size_t start = compiler->buffer->count;
	compiler_add_int(compiler, 0);
	compiler_add_int(compiler, 0);

	enter_scope(compiler);
	compiler->leftmost_pattern = true;
	visit(compiler, patterns->children[curr]);
	int64_t body_start = compiler->buffer->count;
	unsigned char bindings = (unsigned char) (in_function(compiler) ? compiler->params->scope->vars.count : compiler->stack->vars.count);
	if (bindings) {
		compiler_add_byte(compiler, O_INCSP);
		compiler_add_byte(compiler, bindings);
	}
	visit(compiler, bodies->children[curr]);
	exit_scope(compiler);

	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, start, body_start - start);

	curr++;
	size_t body_end = 0;
	if (patterns->children_len > curr) {
		enter_jump(compiler, &body_end);
	}

	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, start + 8, compiler->buffer->count - start);

	if (patterns->children_len <= curr) return;

	visit_Match_helper(compiler, patterns, bodies, curr);

	if (patterns->children_len > curr) {
		exit_jump(compiler, &body_end);
	}
}

static void visit_Match(struct Compiler *const compiler, const struct Node *const node) {
	struct Node *expr = Match_get_expr(node);
	struct Node *patterns = Match_get_patterns(node);
	struct Node *bodies = Match_get_bodies(node);
	visit(compiler, expr);
	if (patterns->children_len == 0) {
		compiler_add_byte(compiler, O_POP);
		return;
	}

	compiler_add_byte(compiler, O_MATCH);
	compiler_add_int(compiler, patterns->children_len);
	visit_Match_helper(compiler, patterns, bodies, 0);
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
	struct Node *else_br = If_get_else(node);

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

static void visit_Print(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Print_get_expr(node));
	compiler_add_byte(compiler, O_PRINT);
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

	struct Scope *scope = compiler->params ? compiler->params->scope : compiler->stack;
	// while (scope && scope->parent) scope = scope->parent;


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
	FOR_CHILDREN(i, child, node) {
		visit(compiler, child);
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
	if (node->value.type == T_DQMARK) {     // ?? operator
		visit_BinOp_shortcircuit(compiler, node, O_BRN_8);
		return;
	} else if (node->value.type == T_DBAR) {  // or operator
		visit_BinOp_shortcircuit(compiler, node, O_BRT_8);
		return;
	} else if (node->value.type == T_DAMP) {   // and operator
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
		puts("error in visit_BinOp");
		exit(EXIT_FAILURE);
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
		YASL_ASSERT(false, "Error in visit_UnOp");
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

static void visit_Float(struct Compiler *const compiler, const struct Node *const node) {
	yasl_float val = Float_get_float(node);
	if (val == 0.0) {
		compiler_add_byte(compiler, O_DCONST_0);
	} else if (val == 1.0) {
		compiler_add_byte(compiler, O_DCONST_1);
	} else if (val == 2.0) {
		compiler_add_byte(compiler, O_DCONST_2);
	} else {
		compiler_add_byte(compiler, O_DCONST);
		YASL_ByteBuffer_add_float(compiler->buffer, val);
	}
}

static void visit_Integer(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int val = Integer_get_int(node);
	YASL_COMPILE_DEBUG_LOG("int64: %"
				       PRId64
				       "\n", val);
	switch (val) {
	case -1:
		compiler_add_byte(compiler, O_ICONST_M1);
		break;
	case 0:
		compiler_add_byte(compiler, O_ICONST_0);
		break;
	case 1:
		compiler_add_byte(compiler, O_ICONST_1);
		break;
	case 2:
		compiler_add_byte(compiler, O_ICONST_2);
		break;
	case 3:
		compiler_add_byte(compiler, O_ICONST_3);
		break;
	case 4:
		compiler_add_byte(compiler, O_ICONST_4);
		break;
	case 5:
		compiler_add_byte(compiler, O_ICONST_5);
		break;
	default:
		if (-(1 << 7) < val && val < (1 << 7)) {
			compiler_add_byte(compiler, O_ICONST_B1);
			compiler_add_byte(compiler, (unsigned char) val);
		} else {
			compiler_add_byte(compiler, O_ICONST_B8);
			compiler_add_int(compiler, val);
		}
		break;
	}
}

static void visit_Boolean(struct Compiler *const compiler, const struct Node *const node) {
	compiler_add_byte(compiler, Boolean_get_bool(node) ? O_BCONST_T : O_BCONST_F);
}

static void visit_String(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int index = intern_string(compiler, node);

	compiler_add_byte(compiler, O_NEWSTR);
	compiler_add_int(compiler, index);
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
	size_t buffer_count = compiler->buffer->count;
	size_t header_count = compiler->header->count;
	size_t code_count = compiler->code->count;
	size_t line_count = compiler->lines->count;
	size_t line = compiler->line;
	visit(compiler, node);
	compiler->buffer->count = buffer_count;
	compiler->header->count = header_count;
	compiler->code->count = code_count;
	compiler->lines->count = line_count;
	compiler->line = line;
}

static void visit(struct Compiler *const compiler, const struct Node *const node) {
	while (node->line > compiler->line) {
		YASL_ByteBuffer_add_vint(compiler->lines, compiler->code->count + compiler->buffer->count);
		compiler->line++;
	}

	switch (node->nodetype) {
	case N_EXPRSTMT:
		visit_ExprStmt(compiler, node);
		break;
	case N_BLOCK:
		visit_Block(compiler, node);
		break;
	case N_BODY:
		visit_Body(compiler, node);
		break;
	case N_FNDECL:
		visit_FunctionDecl(compiler, node);
		break;
	case N_RET:
		visit_Return(compiler, node);
		break;
	case N_EXPORT:
		visit_Export(compiler, node);
		break;
	case N_CALL:
		visit_Call(compiler, node);
		break;
	case N_MCALL:
		visit_MethodCall(compiler, node);
		break;
	case N_SET:
		visit_Set(compiler, node);
		break;
	case N_GET:
		visit_Get(compiler, node);
		break;
	case N_SLICE:
		visit_Slice(compiler, node);
		break;
	case N_LETITER:
		exit(1);
		break;
	case N_LISTCOMP:
		visit_ListComp(compiler, node);
		break;
	case N_TABLECOMP:
		visit_TableComp(compiler, node);
		break;
	case N_FORITER:
		visit_ForIter(compiler, node);
		break;
	case N_WHILE:
		visit_While(compiler, node);
		break;
	case N_BREAK:
		visit_Break(compiler, node);
		break;
	case N_CONT:
		visit_Continue(compiler, node);
		break;
	case N_MATCH:
		visit_Match(compiler, node);
		break;
	case N_PATUNDEF:
		visit_UndefPattern(compiler, node);
		break;
	case N_PATBOOL:
		visit_BoolPattern(compiler, node);
		break;
	case N_PATFL:
		visit_FloatPattern(compiler, node);
		break;
	case N_PATINT:
		visit_IntPattern(compiler, node);
		break;
	case N_PATSTR:
		visit_StringPattern(compiler, node);
		break;
	case N_PATTABLE:
		visit_TablePattern(compiler, node);
		break;
	case N_PATLS:
		visit_ListPattern(compiler, node);
		break;
	case N_PATVTABLE:
		visit_VarTablePattern(compiler, node);
		break;
	case N_PATVLS:
		visit_VarListPattern(compiler, node);
		break;
	case N_PATLET:
		visit_LetPattern(compiler, node);
		break;
	case N_PATCONST:
		visit_ConstPattern(compiler, node);
		break;
	case N_PATANY:
		visit_AnyPattern(compiler, node);
		break;
	case N_PATALT:
		visit_AltPattern(compiler, node);
		break;
	case N_IF:
		visit_If(compiler, node);
		break;
	case N_PRINT:
		visit_Print(compiler, node);
		break;
	case N_LET:
		visit_Let(compiler, node);
		break;
	case N_CONST:
		visit_Const(compiler, node);
		break;
	case N_DECL:
		visit_Decl(compiler, node);
		break;
	case N_TRIOP:
		visit_TriOp(compiler, node);
		break;
	case N_BINOP:
		visit_BinOp(compiler, node);
		break;
	case N_UNOP:
		visit_UnOp(compiler, node);
		break;
	case N_ASSIGN:
		visit_Assign(compiler, node);
		break;
	case N_LIST:
		visit_List(compiler, node);
		break;
	case N_TABLE:
		visit_Table(compiler, node);
		break;
	case N_VAR:
		visit_Var(compiler, node);
		break;
	case N_UNDEF:
		visit_Undef(compiler, node);
		break;
	case N_FLOAT:
		visit_Float(compiler, node);
		break;
	case N_INT:
		visit_Integer(compiler, node);
		break;
	case N_BOOL:
		visit_Boolean(compiler, node);
		break;
	case N_STR:
		visit_String(compiler, node);
		break;
	case N_ASS:
		visit_Assert(compiler, node);
		break;
	}
}
