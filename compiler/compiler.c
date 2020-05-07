#include "compiler.h"

#include <math.h>
#include <interpreter/YASL_Object.h>

#include "ast.h"
#include "data-structures/YASL_String.h"
#include "lexinput.h"
#include "opcode.h"
#include "yasl_error.h"
#include "yasl_include.h"

#define compiler_print_err(compiler, format, ...) {\
	char *tmp = (char *)malloc(snprintf(NULL, 0, format, __VA_ARGS__) + 1);\
	sprintf(tmp, format, __VA_ARGS__);\
	(compiler)->parser.lex.err.print(&(compiler)->parser.lex.err, tmp, strlen(tmp));\
	free(tmp);\
}

#define compiler_print_err_syntax(compiler, format, ...) compiler_print_err(compiler, "SyntaxError: " format, __VA_ARGS__)

#define compiler_print_err_undeclared_var(compiler, name, line) \
	compiler_print_err_syntax((compiler), "Undeclared variable %s (line %" PRI_SIZET ").\n", name, line)

#define compiler_print_err_const(compiler, name, line) \
	compiler_print_err_syntax((compiler), "Cannot assign to constant %s (line %" PRI_SIZET ").\n", name, line)

#define break_checkpoint(compiler)    ((compiler)->checkpoints.items[(compiler)->checkpoints.count-1])
#define continue_checkpoint(compiler) ((compiler)->checkpoints.items[(compiler)->checkpoints.count-2])


static enum SpecialStrings get_special_string(const struct Node *const node) {
#define STR_EQ(node, literal) ((node)->value.sval.str_len == strlen((literal)) && !memcmp((node)->value.sval.str, (literal), (node)->value.sval.str_len))
	if (STR_EQ(node, "__add")) return S___ADD;
	else if (STR_EQ(node, "__get")) return S___GET;
	else if (STR_EQ(node, "__set")) return S___SET;
	else if (STR_EQ(node, "clear")) return S_CLEAR;
	else if (STR_EQ(node, "copy")) return S_COPY;
	else if (STR_EQ(node, "endswith")) return S_ENDSWITH;
	else if (STR_EQ(node, "extend")) return S_EXTEND;
	else if (STR_EQ(node, "isal")) return S_ISAL;
	else if (STR_EQ(node, "isalnum")) return S_ISALNUM;
	else if (STR_EQ(node, "isnum")) return S_ISNUM;
	else if (STR_EQ(node, "isspace")) return S_ISSPACE;
	else if (STR_EQ(node, "join")) return S_JOIN;
	else if (STR_EQ(node, "sort")) return S_SORT;
	else if (STR_EQ(node, "keys")) return S_KEYS;
	else if (STR_EQ(node, "ltrim")) return S_LTRIM;
	else if (STR_EQ(node, "pop")) return S_POP;
	else if (STR_EQ(node, "push")) return S_PUSH;
	else if (STR_EQ(node, "remove")) return S_REMOVE;
	else if (STR_EQ(node, "rep")) return S_REP;
	else if (STR_EQ(node, "replace")) return S_REPLACE;
	else if (STR_EQ(node, "reverse")) return S_REVERSE;
	else if (STR_EQ(node, "rtrim")) return S_RTRIM;
	else if (STR_EQ(node, "search")) return S_SEARCH;
	// else if (STR_EQ(node, "slice")) return S_SLICE;
	else if (STR_EQ(node, "split")) return S_SPLIT;
	else if (STR_EQ(node, "startswith")) return S_STARTSWITH;
	else if (STR_EQ(node, "tobool")) return S_TOBOOL;
	else if (STR_EQ(node, "tofloat")) return S_TOFLOAT;
	else if (STR_EQ(node, "toint")) return S_TOINT;
	else if (STR_EQ(node, "tolower")) return S_TOLOWER;
	else if (STR_EQ(node, "tostr")) return S_TOSTR;
	else if (STR_EQ(node, "toupper")) return S_TOUPPER;
	else if (STR_EQ(node, "trim")) return S_TRIM;
	else if (STR_EQ(node, "values")) return S_VALUES;
	else return S_UNKNOWN_STR;
#undef STR_EQ
}

void compiler_tables_del(struct Compiler *compiler) {
	YASL_Table_del(compiler->strings);
}

static void compiler_buffers_del(const struct Compiler *const compiler) {
	YASL_ByteBuffer_del(compiler->buffer);
	YASL_ByteBuffer_del(compiler->header);
	YASL_ByteBuffer_del(compiler->code);
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
			YASL_ByteBuffer_add_byte(compiler->buffer, O_POP);
		}
	} else {
		struct Scope *tmp = compiler->stack;
		compiler->stack = compiler->stack->parent;
		scope_del_current_only(tmp);
	}
}

static inline void enter_conditional_false(const struct Compiler *const compiler, int64_t *const index) {
	YASL_ByteBuffer_add_byte(compiler->buffer, O_BRF_8);
	*index = compiler->buffer->count;
	YASL_ByteBuffer_add_int(compiler->buffer, 0);
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
	if (compiler->params && scope_contains(compiler->params->scope, name)) {
		int64_t index = get_index(scope_get(compiler->params->scope, name));
		YASL_ByteBuffer_add_byte(compiler->buffer, O_LLOAD_1);
		YASL_ByteBuffer_add_byte(compiler->buffer, (unsigned char) index);
	} else if (env_contains(compiler->params, name)) {
		compiler->params->isclosure = true;
		// TODO fix this, we need to handle arbitrarily nested closures
		// compiler->params->parent->usedinclosure = true;
		// I Think I got it now?
		struct Env *curr = compiler->params;
		while (!env_contains_cur_only(curr, name)) {
			curr = curr->parent;
		}
		curr->usedinclosure = true;
		YASL_ByteBuffer_add_byte(compiler->buffer, O_ULOAD_1);
		yasl_int tmp = env_resolve_upval_index(compiler->params, name);
		YASL_ByteBuffer_add_byte(compiler->buffer, (unsigned char) tmp);
	} else if (scope_contains(compiler->stack, name)) {
		int64_t index = get_index(scope_get(compiler->stack, name));
		YASL_ByteBuffer_add_byte(compiler->buffer, O_GLOAD_1);
		YASL_ByteBuffer_add_byte(compiler->buffer, (unsigned char) index);
	} else if (scope_contains(compiler->globals, name)) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_GLOAD_8);
		YASL_ByteBuffer_add_int(compiler->buffer, compiler->num);
		YASL_ByteBuffer_add_int(compiler->buffer,
					YASL_Table_search_string_int(compiler->strings, name, name_len).value.ival);
	} else {
		compiler_print_err_undeclared_var(compiler, name, line);
		handle_error(compiler);
		return;
	}
}

static void store_var(struct Compiler *const compiler, const char *const name, const size_t line) {
	const size_t name_len = strlen(name);
	if (compiler->params && scope_contains(compiler->params->scope, name)) {
		int64_t index = scope_get(compiler->params->scope, name);
		if (is_const(index)) {
			compiler_print_err_const(compiler, name, line);
			handle_error(compiler);
			return;
		}
		YASL_ByteBuffer_add_byte(compiler->buffer, O_LSTORE_1);
		YASL_ByteBuffer_add_byte(compiler->buffer, (unsigned char) index);
	} else if (compiler->params && compiler->params->parent && scope_contains(compiler->params->parent->scope, name)) {
		compiler->params->isclosure = true;
		// TODO fix this, we need to handle arbitrarily nested closures
		compiler->params->parent->usedinclosure = true;
		YASL_ByteBuffer_add_byte(compiler->buffer, O_USTORE_1);
		YASL_ByteBuffer_add_byte(compiler->buffer, (unsigned char) env_resolve_upval_index(compiler->params, name));
	} else if (scope_contains(compiler->stack, name)) {
		int64_t index = scope_get(compiler->stack, name);
		if (is_const(index)) {
			compiler_print_err_const(compiler, name, line);
			handle_error(compiler);
			return;
		}
		YASL_ByteBuffer_add_byte(compiler->buffer, O_GSTORE_1);
		YASL_ByteBuffer_add_byte(compiler->buffer, (unsigned char) index);
	} else if (scope_contains(compiler->globals, name)) {
		int64_t index = scope_get(compiler->globals, name);
		if (is_const(index)) {
			compiler_print_err_const(compiler, name, line);
			handle_error(compiler);
			return;
		}
		YASL_ByteBuffer_add_byte(compiler->buffer, O_GSTORE_8);
		YASL_ByteBuffer_add_int(compiler->buffer, compiler->num);
		YASL_ByteBuffer_add_int(compiler->buffer,
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
			YASL_PRINT_ERROR_TOO_MANY_VAR(line);
			handle_error(compiler);
		}
	} else if (compiler->stack) {
		int64_t index = scope_decl_var(compiler->stack, name);
		if (index > 255) {
			YASL_PRINT_ERROR_TOO_MANY_VAR(line);
			handle_error(compiler);
		}
	} else {
		struct YASL_Object value = YASL_Table_search_string_int(compiler->strings, name, name_len);
		if (value.type == Y_END) {
			YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
			YASL_Table_insert_string_int(compiler->strings, name, name_len, compiler->header->count);
			YASL_ByteBuffer_add_int(compiler->header, name_len);
			YASL_ByteBuffer_extend(compiler->header, (unsigned char *) name, name_len);
		}
		scope_decl_var(compiler->globals, name);
		//if (index > 255) {
		//	YASL_PRINT_ERROR_TOO_MANY_VAR(line);
		//	handle_error(compiler);
		//}
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
	YASL_ByteBuffer_rewrite_int_fast(compiler->header, 8, compiler->code->count + compiler->header->count +
							      1);   // TODO: put num globals here eventually.

	YASL_BYTECODE_DEBUG_LOG("%s\n", "header");
	for (size_t i = 0; i < compiler->header->count; i++) {
		if (i % 16 == 15)
			YASL_BYTECODE_DEBUG_LOG("%02x\n", compiler->header->bytes[i]);
		else
			YASL_BYTECODE_DEBUG_LOG("%02x ", compiler->header->bytes[i]);
	}
	YASL_BYTECODE_DEBUG_LOG("%s\n", "entry point");
	for (size_t i = 0; i < compiler->code->count; i++) {
		if (i % 16 == 15)
			YASL_BYTECODE_DEBUG_LOG("%02x\n", compiler->code->bytes[i]);
		else
			YASL_BYTECODE_DEBUG_LOG("%02x ", compiler->code->bytes[i]);
	}
	YASL_BYTECODE_DEBUG_LOG("%02x\n", O_HALT);

	fflush(stdout);
	unsigned char *bytecode = (unsigned char *) malloc(
		compiler->code->count + compiler->header->count + 1);    // NOT OWN
	memcpy(bytecode, compiler->header->bytes, compiler->header->count);
	memcpy(bytecode + compiler->header->count, compiler->code->bytes, compiler->code->count);
	bytecode[compiler->code->count + compiler->header->count] = O_HALT;
	return bytecode;
}

unsigned char *compile(struct Compiler *const compiler) {
	struct Node *node;
	gettok(&compiler->parser.lex);
	while (!peof(&compiler->parser)) {
		if (peof(&compiler->parser)) break;
		node = parse(&compiler->parser);
		if (compiler->parser.status) {
			compiler->status |= compiler->parser.status;
			return NULL;
		}
		eattok(&compiler->parser, T_SEMI);
		visit(compiler, node);
		YASL_ByteBuffer_extend(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
		compiler->buffer->count = 0;

		node_del(node);
	}

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
	size_t curr = compiler->buffer->count;
	switch (expr->nodetype) {
	case N_STR:
	case N_INT:
	case N_FLOAT:
	case N_BOOL:
	case N_UNDEF:
		return;
	case N_VAR:
		visit(compiler, expr);
		compiler->buffer->count = curr;
		return;
	default:
		visit(compiler, expr);
		if (expr->nodetype == N_ASSIGN || expr->nodetype == N_SET) {
			return;
		} else {
			YASL_ByteBuffer_add_byte(compiler->buffer, O_POP);
		}
	}
}

static void visit_FunctionDecl(struct Compiler *const compiler, const struct Node *const node) {
	/*if (in_function(compiler)) {
		YASL_PRINT_ERROR_SYNTAX("Illegal function declaration outside global scope, in line %" PRI_SIZET ".\n",
					node->line);
		handle_error(compiler);
		return;
	}*/

	// start logic for function, now that we are sure it's legal to do so, and have set up.
	compiler->params = env_new(compiler->params);

	enter_scope(compiler);

	FOR_CHILDREN(i, child, FnDecl_get_params(node)) {
		decl_var(compiler, Decl_get_name(child), child->line);
		if (child->nodetype == N_CONST) {
			make_const(compiler, Decl_get_name(child));
		}
	}

	size_t old_size = compiler->buffer->count;
	// TODO: verfiy that number of params is small enough. (same for the other casts below.)

	YASL_ByteBuffer_add_byte(compiler->buffer, (unsigned char) Body_get_len(FnDecl_get_params(node)));
	YASL_ByteBuffer_add_byte(compiler->buffer, 0);  // TODO: remove this
	visit_Body(compiler, FnDecl_get_body(node));
	exit_scope(compiler);

	int64_t fn_val = compiler->header->count;
	YASL_ByteBuffer_extend(compiler->header, compiler->buffer->bytes + old_size, compiler->buffer->count - old_size);
	compiler->buffer->count = old_size;
	YASL_ByteBuffer_add_byte(compiler->header, O_NCONST);
	YASL_ByteBuffer_add_byte(compiler->header, compiler->params->usedinclosure ? O_CRET : O_RET);

	// zero buffer length
	compiler->buffer->count = old_size;

	if (!compiler->params->isclosure) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_FCONST);
		YASL_ByteBuffer_add_int(compiler->buffer, fn_val);
	} else {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_CCONST);
		YASL_ByteBuffer_add_int(compiler->buffer, fn_val);
		const size_t count = compiler->params->upval_indices.count;
		YASL_ByteBuffer_add_byte(compiler->buffer, (unsigned char) count);
		const size_t start = compiler->buffer->count;
		// TODO what's below is wrong. We need to get the right value for the upvals.
		for (size_t i = 0; i < count; i++) {
			YASL_ByteBuffer_add_byte(compiler->buffer, 0);
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
	YASL_ByteBuffer_add_byte(compiler->buffer, O_INIT_CALL);
	visit_Body(compiler, Call_get_params(node));
	YASL_ByteBuffer_add_byte(compiler->buffer, O_CALL);
}

static void visit_MethodCall(struct Compiler *const compiler, const struct Node *const node) {
	char *str = MCall_get_name(node);
	size_t len = strlen(str);
	visit(compiler, Call_get_object(node));
	enum SpecialStrings index = get_special_string(node);
	if (index != S_UNKNOWN_STR) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_INIT_MC_SPECIAL);
		YASL_ByteBuffer_add_byte(compiler->buffer, index);
	} else {
		struct YASL_Object value = YASL_Table_search_string_int(compiler->strings, str, len);
		if (value.type == Y_END) {
			YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
			YASL_Table_insert_string_int(compiler->strings, str, len, compiler->header->count);
			YASL_ByteBuffer_add_int(compiler->header, len);
			YASL_ByteBuffer_extend(compiler->header, (unsigned char *) str, len);
		}

		value = YASL_Table_search_string_int(compiler->strings, str, len);

		YASL_ByteBuffer_add_byte(compiler->buffer, O_INIT_MC);
		YASL_ByteBuffer_add_int(compiler->buffer, compiler->num);
		YASL_ByteBuffer_add_int(compiler->buffer, value.value.ival);
	}

	visit_Body(compiler, Call_get_params(node));
	YASL_ByteBuffer_add_byte(compiler->buffer, O_CALL);
}

static void visit_Return(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Return_get_expr(node));
	YASL_ByteBuffer_add_byte(compiler->buffer, compiler->params->usedinclosure ? O_CRET : O_RET);
}

static void visit_Export(struct Compiler *const compiler, const struct Node *const node) {
	if (compiler->stack != NULL || compiler->params != NULL) {
		YASL_PRINT_ERROR("export statement must be at top level of module. (line %" PRI_SIZET ")\n", node->line);
		handle_error(compiler);
		return;
	}
	visit(compiler, Export_get_expr(node));
	YASL_ByteBuffer_add_byte(compiler->buffer, O_EXPORT);
}

static void visit_Set(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Set_get_collection(node));
	visit(compiler, Set_get_key(node));
	visit(compiler, Set_get_value(node));
	YASL_ByteBuffer_add_byte(compiler->buffer, O_SET);
}

static void visit_Get(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Get_get_collection(node));
	visit(compiler, Get_get_value(node));
	YASL_ByteBuffer_add_byte(compiler->buffer, O_GET);
}

static void visit_Slice(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Slice_get_collection(node));
	visit(compiler, Slice_get_start(node));
	visit(compiler, Slice_get_end(node));
	YASL_ByteBuffer_add_byte(compiler->buffer, O_SLICE);
}

static void visit_Block(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);
	visit(compiler, Block_get_block(node));
	exit_scope(compiler);
}

static inline void branch_back(struct Compiler *const compiler, int64_t index) {
	YASL_ByteBuffer_add_byte(compiler->buffer, O_BR_8);
	YASL_ByteBuffer_add_int(compiler->buffer, index - compiler->buffer->count - 8);
}

static void visit_ListComp_cond(struct Compiler *const compiler, const struct Node *const cond, const struct Node *const expr) {
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

static void visit_TableComp_cond(struct Compiler *const compiler, const struct Node *const cond, const struct Node *const expr) {
	if (cond) {
		int64_t index_third;
		visit(compiler, cond);
		enter_conditional_false(compiler, &index_third);

		visit(compiler, expr->children[0]);
		visit(compiler, expr->children[1]);

		exit_conditional_false(compiler, &index_third);
	} else {
		visit(compiler, expr->children[0]);
		visit(compiler, expr->children[1]);
	}
}

static void visit_ListComp(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);

	struct Node *expr = ListComp_get_expr(node);
	struct Node *cond = ListComp_get_cond(node);
	struct Node *iter = ListComp_get_iter(node);

	struct Node *collection = LetIter_get_collection(iter);

	char *name = iter->value.sval.str;

	visit(compiler, collection);

	YASL_ByteBuffer_add_byte(compiler->buffer, O_INITFOR);
	YASL_ByteBuffer_add_byte(compiler->buffer, O_END);
	decl_var(compiler, name, iter->line);
	YASL_ByteBuffer_add_byte(compiler->buffer, O_END);

	int64_t index_start = compiler->buffer->count;

	YASL_ByteBuffer_add_byte(compiler->buffer, O_ITER_1);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, name, iter->line);

	visit_ListComp_cond(compiler, cond, expr);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	YASL_ByteBuffer_add_byte(compiler->buffer, O_NEWLIST);

	YASL_ByteBuffer_add_byte(compiler->buffer, O_ENDCOMP);

	size_t curr = compiler->buffer->count;
	exit_scope(compiler);
	compiler->buffer->count = curr;
}

static void visit_TableComp(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);

	struct Node *expr = TableComp_get_key_value(node);
	struct Node *iter = TableComp_get_iter(node);
	struct Node *cond = TableComp_get_cond(node);

	struct Node *collection = LetIter_get_collection(iter);
	char *name = iter->value.sval.str;

	visit(compiler, collection);

	YASL_ByteBuffer_add_byte(compiler->buffer, O_INITFOR);
	YASL_ByteBuffer_add_byte(compiler->buffer, O_END);
	decl_var(compiler, name, iter->line);
	YASL_ByteBuffer_add_byte(compiler->buffer, O_END);

	int64_t index_start = compiler->buffer->count;

	YASL_ByteBuffer_add_byte(compiler->buffer, O_ITER_1);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, name, iter->line);

	visit_TableComp_cond(compiler, cond, expr);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	YASL_ByteBuffer_add_byte(compiler->buffer, O_NEWTABLE);
	YASL_ByteBuffer_add_byte(compiler->buffer, O_ENDCOMP);

	size_t curr = compiler->buffer->count;
	exit_scope(compiler);
	compiler->buffer->count = curr;
}

static void visit_ForIter(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);

	struct Node *iter = ForIter_get_iter(node);
	struct Node *body = ForIter_get_body(node);

	struct Node *collection = LetIter_get_collection(iter);
	char *name = iter->value.sval.str;

	visit(compiler, collection);

	YASL_ByteBuffer_add_byte(compiler->buffer, O_INITFOR);
	YASL_ByteBuffer_add_byte(compiler->buffer, O_END);
	decl_var(compiler, name, iter->line);

	size_t index_start = compiler->buffer->count;
	add_checkpoint(compiler, index_start);

	YASL_ByteBuffer_add_byte(compiler->buffer, O_ITER_1);

	add_checkpoint(compiler, compiler->buffer->count);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, name, iter->line);

	visit(compiler, body);

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	YASL_ByteBuffer_add_byte(compiler->buffer, O_ENDFOR);
	exit_scope(compiler);

	rm_checkpoint(compiler);
	rm_checkpoint(compiler);
}

static void enter_jump(struct Compiler *const compiler, size_t *index) {
	YASL_ByteBuffer_add_byte(compiler->buffer, O_BR_8);
	*index = compiler->buffer->count;
	YASL_ByteBuffer_add_int(compiler->buffer, 0);
}

static void exit_jump(struct Compiler *const compiler, const size_t *const index) {
	YASL_ByteBuffer_rewrite_int_fast(compiler->buffer, *index, compiler->buffer->count - *index - 8);
}

static void visit_While(struct Compiler *const compiler, const struct Node *const node) {
	size_t index_start = compiler->buffer->count;

	struct Node *cond = While_get_cond(node);
	struct Node *body = While_get_body(node);
	struct Node *post = While_get_post(node);

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
		YASL_PRINT_ERROR_SYNTAX("break outside of loop (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}
	YASL_ByteBuffer_add_byte(compiler->buffer, O_BCONST_F);
	branch_back(compiler, break_checkpoint(compiler));
}

static void visit_Continue(struct Compiler *const compiler, const struct Node *const node) {
	if (compiler->checkpoints.count == 0) {
		YASL_PRINT_ERROR_SYNTAX("continue outside of loop (line %" PRI_SIZET ").\n", node->line);
		handle_error(compiler);
		return;
	}
	branch_back(compiler, continue_checkpoint(compiler));
}

static bool Node_istruthy(const struct Node *const node) {
	switch (node->nodetype)	{
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

static void visit_If_true(struct Compiler *const compiler, const struct Node *const then_br, const struct Node *const else_br) {
	visit(compiler, then_br);

	if (else_br) {
		size_t curr = compiler->buffer->count;
		visit(compiler, else_br);
		compiler->buffer->count = curr;
	}
}

static void visit_If_false(struct Compiler *const compiler, const struct Node *const then_br, const struct Node *const else_br) {
	size_t curr = compiler->buffer->count;
	visit(compiler, then_br);
	compiler->buffer->count = curr;

	if (else_br) {
		visit(compiler, else_br);
	}
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
	YASL_ByteBuffer_add_byte(compiler->buffer, O_PRINT);
}

static void declare_with_let_or_const(struct Compiler *const compiler, const struct Node *const node) {
	if (contains_var_in_current_scope(compiler, Decl_get_name(node))) {
		compiler_print_err_syntax(compiler, "Illegal redeclaration of %s (line %" PRI_SIZET ").\n", Decl_get_name(node), node->line);
		handle_error(compiler);
		return;
	}

	if (Decl_get_expr(node) && Decl_get_expr(node)->nodetype == N_FNDECL) {
		decl_var(compiler, Decl_get_name(node), node->line);
		visit(compiler, Decl_get_expr(node));
	} else {
		if (Decl_get_expr(node)) visit(compiler, Decl_get_expr(node));
		else YASL_ByteBuffer_add_byte(compiler->buffer, O_NCONST);

		decl_var(compiler, Decl_get_name(node), node->line);
	}

	if (!compiler->params || !(scope_contains(compiler->params->scope, Decl_get_name(node)))) {
		store_var(compiler, Decl_get_name(node), node->line);
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
	YASL_ByteBuffer_add_byte(compiler->buffer, O_DUP);
	YASL_ByteBuffer_add_byte(compiler->buffer, jump_type);
	size_t index = compiler->buffer->count;
	YASL_ByteBuffer_add_int(compiler->buffer, 0);
	YASL_ByteBuffer_add_byte(compiler->buffer, O_POP);
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
	case T_BAR: YASL_ByteBuffer_add_byte(compiler->buffer, O_BOR);
		break;
	case T_CARET: YASL_ByteBuffer_add_byte(compiler->buffer, O_BXOR);
		break;
	case T_AMP: YASL_ByteBuffer_add_byte(compiler->buffer, O_BAND);
		break;
	case T_AMPCARET: YASL_ByteBuffer_add_byte(compiler->buffer, O_BANDNOT);
		break;
	case T_DEQ: YASL_ByteBuffer_add_byte(compiler->buffer, O_EQ);
		break;
	case T_TEQ: YASL_ByteBuffer_add_byte(compiler->buffer, O_ID);
		break;
	case T_BANGEQ: YASL_ByteBuffer_add_byte(compiler->buffer, O_EQ);
		YASL_ByteBuffer_add_byte(compiler->buffer, O_NOT);
		break;
	case T_BANGDEQ: YASL_ByteBuffer_add_byte(compiler->buffer, O_ID);
		YASL_ByteBuffer_add_byte(compiler->buffer, O_NOT);
		break;
	case T_GT: YASL_ByteBuffer_add_byte(compiler->buffer, O_GT);
		break;
	case T_GTEQ: YASL_ByteBuffer_add_byte(compiler->buffer, O_GE);
		break;
	case T_LT: YASL_ByteBuffer_add_byte(compiler->buffer, O_GE);
		YASL_ByteBuffer_add_byte(compiler->buffer, O_NOT);
		break;
	case T_LTEQ: YASL_ByteBuffer_add_byte(compiler->buffer, O_GT);
		YASL_ByteBuffer_add_byte(compiler->buffer, O_NOT);
		break;
	case T_TILDE: YASL_ByteBuffer_add_byte(compiler->buffer, O_CNCT);
		break;
	case T_DGT: YASL_ByteBuffer_add_byte(compiler->buffer, O_BSR);
		break;
	case T_DLT: YASL_ByteBuffer_add_byte(compiler->buffer, O_BSL);
		break;
	case T_PLUS: YASL_ByteBuffer_add_byte(compiler->buffer, O_ADD);
		break;
	case T_MINUS: YASL_ByteBuffer_add_byte(compiler->buffer, O_SUB);
		break;
	case T_STAR: YASL_ByteBuffer_add_byte(compiler->buffer, O_MUL);
		break;
	case T_SLASH: YASL_ByteBuffer_add_byte(compiler->buffer, O_FDIV);
		break;
	case T_DSLASH: YASL_ByteBuffer_add_byte(compiler->buffer, O_IDIV);
		break;
	case T_MOD: YASL_ByteBuffer_add_byte(compiler->buffer, O_MOD);
		break;
	case T_DSTAR: YASL_ByteBuffer_add_byte(compiler->buffer, O_EXP);
		break;
	default:
		puts("error in visit_BinOp");
		exit(EXIT_FAILURE);
	}
}

static void visit_UnOp(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, UnOp_get_expr(node));
	switch (node->value.type) {
	case T_PLUS: YASL_ByteBuffer_add_byte(compiler->buffer, O_POS);
		break;
	case T_MINUS: YASL_ByteBuffer_add_byte(compiler->buffer, O_NEG);
		break;
	case T_BANG: YASL_ByteBuffer_add_byte(compiler->buffer, O_NOT);
		break;
	case T_CARET: YASL_ByteBuffer_add_byte(compiler->buffer, O_BNOT);
		break;
	case T_LEN: YASL_ByteBuffer_add_byte(compiler->buffer, O_LEN);
		break;
	default:
		puts("error in visit_UnOp");
		exit(EXIT_FAILURE);
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
	YASL_ByteBuffer_add_byte(compiler->buffer, O_NCONST);
}

static void visit_Float(struct Compiler *const compiler, const struct Node *const node) {
	yasl_float val = Float_get_float(node);
	if (val == 0.0) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_DCONST_0);
	} else if (val == 1.0) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_DCONST_1);
	} else if (val == 2.0) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_DCONST_2);
	} else if (val != val) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_DCONST_N);
	} else if (isinf(val)) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_DCONST_I);
	} else {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_DCONST);
		YASL_ByteBuffer_add_float(compiler->buffer, val);
	}
}

static void visit_Integer(struct Compiler *const compiler, const struct Node *const node) {
	yasl_int val = Integer_get_int(node);
	YASL_COMPILE_DEBUG_LOG("int64: %" PRId64 "\n", val);
	switch (val) {
	case -1: YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST_M1);
		break;
	case 0: YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST_0);
		break;
	case 1: YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST_1);
		break;
	case 2: YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST_2);
		break;
	case 3: YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST_3);
		break;
	case 4: YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST_4);
		break;
	case 5: YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST_5);
		break;
	default:
		if (-(1 << 7) < val && val < (1 << 7)) {
			YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST_B1);
			YASL_ByteBuffer_add_byte(compiler->buffer, val);
		} else {
			YASL_ByteBuffer_add_byte(compiler->buffer, O_ICONST);
			YASL_ByteBuffer_add_int(compiler->buffer, val);
		}
		break;
	}
}

static void visit_Boolean(struct Compiler *const compiler, const struct Node *const node) {
	YASL_ByteBuffer_add_byte(compiler->buffer, Boolean_get_bool(node) ? O_BCONST_T : O_BCONST_F);
}

static void visit_String(struct Compiler *const compiler, const struct Node *const node) {
	const char *const str = String_get_str(node);
	size_t len = String_get_len(node);

	struct YASL_Object value = YASL_Table_search_string_int(compiler->strings, str, len);
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
		YASL_Table_insert_string_int(compiler->strings, str, len, compiler->header->count);
		YASL_ByteBuffer_add_int(compiler->header, len);
		YASL_ByteBuffer_extend(compiler->header, (unsigned char *) str, len);
	}

	value = YASL_Table_search_string_int(compiler->strings, str, len);

	enum SpecialStrings index = get_special_string(node);
	if (index != S_UNKNOWN_STR) {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_NEWSPECIALSTR);
		YASL_ByteBuffer_add_byte(compiler->buffer, index);
	} else {
		YASL_ByteBuffer_add_byte(compiler->buffer, O_NEWSTR);
		YASL_ByteBuffer_add_int(compiler->buffer, compiler->num);
		YASL_ByteBuffer_add_int(compiler->buffer, value.value.ival);
	}
}

static void make_new_collection(struct Compiler *const compiler, const struct Node *const node, enum Opcode type) {
	YASL_ByteBuffer_add_byte(compiler->buffer, O_END);
	visit_Body(compiler, node);
	YASL_ByteBuffer_add_byte(compiler->buffer, type);
}

static void visit_List(struct Compiler *const compiler, const struct Node *const node) {
	make_new_collection(compiler, List_get_values(node), O_NEWLIST);
}

static void visit_Table(struct Compiler *const compiler, const struct Node *const node) {
	make_new_collection(compiler, Table_get_values(node), O_NEWTABLE);
}

static void visit(struct Compiler *const compiler, const struct Node *const node) {
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
	}
	//jumptable[node->nodetype](compiler, node);
}
