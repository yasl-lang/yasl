#include "compiler.h"

#include "ast.h"
#include "interpreter/YASL_Object.h"
#include "middleend.h"
#include "interpreter/YASL_string.h"
#include "bytebuffer/bytebuffer.h"
#include "parser.h"
#include "yasl_error.h"
#include "yasl_include.h"
#include "lexinput.h"

#include <math.h>

#define break_checkpoint(compiler)    ((compiler)->checkpoints[(compiler)->checkpoints_count-1])
#define continue_checkpoint(compiler) ((compiler)->checkpoints[(compiler)->checkpoints_count-2])


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
	else if (STR_EQ(node, "rep")) return S_REP;
	else if (STR_EQ(node, "replace")) return S_REPLACE;
	else if (STR_EQ(node, "reverse")) return S_REVERSE;
	else if (STR_EQ(node, "rtrim")) return S_RTRIM;
	else if (STR_EQ(node, "search")) return S_SEARCH;
	else if (STR_EQ(node, "slice")) return S_SLICE;
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

/*
 * Initialise everything in the compiler except the parser
 */
static void *init_compiler(struct Compiler *compiler) {
	compiler->globals = env_new(NULL);
	compiler->stack = NULL;
	compiler->params = NULL;

	compiler->num = 0;
	compiler->strings = table_new();
	compiler->buffer = bb_new(16);
	compiler->header = bb_new(16);
	compiler->header->count = 16;
	compiler->status = YASL_SUCCESS;
	compiler->checkpoints_size = 4;
	compiler->checkpoints = (size_t *)malloc(sizeof(size_t) * compiler->checkpoints_size);
	compiler->checkpoints_count = 0;
	compiler->code = bb_new(16);
	return compiler;
}

struct Compiler *compiler_new(FILE *const fp) {
	struct Compiler *compiler = (struct Compiler *)malloc(sizeof(struct Compiler));
	init_compiler(compiler);

	struct LEXINPUT *lp = lexinput_new_file(fp);
	compiler->parser = NEW_PARSER(lp);
	return compiler;
}


struct Compiler *compiler_new_bb(const char *const buf, const size_t len) {
	struct Compiler *compiler = (struct Compiler *)malloc(sizeof(struct Compiler));
	init_compiler(compiler);

	struct LEXINPUT *lp = lexinput_new_bb(buf, len);
	compiler->parser = NEW_PARSER(lp);
	return compiler;
}



void compiler_tables_del(struct Compiler *compiler) {
	table_del_string_int(compiler->strings);
}

static void compiler_buffers_del(const struct Compiler *const compiler) {
	bb_del(compiler->buffer);
	bb_del(compiler->header);
	bb_del(compiler->code);
}

void compiler_cleanup(struct Compiler *const compiler) {
	compiler_tables_del(compiler);
	env_del(compiler->globals);
	env_del(compiler->stack);
	env_del(compiler->params);
	parser_cleanup(&compiler->parser);
	compiler_buffers_del(compiler);
	free(compiler->checkpoints);
}

static void handle_error(struct Compiler *const compiler) {
	compiler->status = YASL_SYNTAX_ERROR;
}

static bool in_function(const struct Compiler *const compiler) {
	return compiler->params != NULL;
}

static void enter_scope(struct Compiler *const compiler) {
	if (in_function(compiler)) compiler->params = env_new(compiler->params);
	else compiler->stack = env_new(compiler->stack);
}

static void exit_scope(struct Compiler *const compiler) {
	if (in_function(compiler)) {
		compiler->num_locals += compiler->params->vars->count;
		Env_t *tmp = compiler->params;
		compiler->params = compiler->params->parent;
		env_del_current_only(tmp);
	} else {
		Env_t *tmp = compiler->stack;
		compiler->stack = compiler->stack->parent;
		env_del_current_only(tmp);
	}
}

static inline void enter_conditional_false(struct Compiler *const compiler, int64_t *const index) {
	bb_add_byte(compiler->buffer, BRF_8);
	*index = compiler->buffer->count;
	bb_intbytes8(compiler->buffer, 0);
}

static inline void exit_conditional_false(struct Compiler *const compiler, const int64_t *const index) {
	bb_rewrite_intbytes8(compiler->buffer, (size_t)*index, compiler->buffer->count - *index - 8);
}

static void add_checkpoint(struct Compiler *const compiler, const size_t cp) {
	if (compiler->checkpoints_count >= compiler->checkpoints_size) {
		compiler->checkpoints_size *= 2;
		compiler->checkpoints = (size_t *)realloc(compiler->checkpoints, sizeof(compiler->checkpoints[0]) * compiler->checkpoints_size);
	}
	compiler->checkpoints[compiler->checkpoints_count++] = cp;
}

static void rm_checkpoint(struct Compiler *compiler) {
	compiler->checkpoints_count--;
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

static void load_var(struct Compiler *const compiler, char *const name, const size_t name_len, const size_t line) {
	if (env_contains(compiler->params, name, name_len)) {
		int64_t index = get_index(env_get(compiler->params, name, name_len));
		bb_add_byte(compiler->buffer, LLOAD_1);
		bb_add_byte(compiler->buffer, (unsigned char)index);
	} else if (env_contains(compiler->stack, name, name_len)) {
		int64_t index = get_index(env_get(compiler->stack, name, name_len));
		bb_add_byte(compiler->buffer, GLOAD_1);
		bb_add_byte(compiler->buffer, (unsigned char) index);
	} else if (env_contains(compiler->globals, name, name_len)) {
		bb_add_byte(compiler->buffer, GLOAD_8);
		bb_intbytes8(compiler->buffer, compiler->num);
		bb_intbytes8(compiler->buffer, table_search_string_int(compiler->strings, name, name_len).value.ival);
	} else {
		YASL_PRINT_ERROR_UNDECLARED_VAR(name, line);
		handle_error(compiler);
		return;
	}
}

static void store_var(struct Compiler *const compiler, char *const name, const size_t name_len, const size_t line) {
	if (env_contains(compiler->params, name, name_len)) {
		int64_t index = env_get(compiler->params, name, name_len);
		if (is_const(index)) {
			YASL_PRINT_ERROR_CONSTANT(name, line);
			handle_error(compiler);
			return;
		}
		bb_add_byte(compiler->buffer, LSTORE_1);
		bb_add_byte(compiler->buffer, (unsigned char) index);
	} else if (env_contains(compiler->stack, name, name_len)) {
		int64_t index = env_get(compiler->stack, name, name_len);
		if (is_const(index)) {
			YASL_PRINT_ERROR_CONSTANT(name, line);
			handle_error(compiler);
			return;
		}
		bb_add_byte(compiler->buffer, GSTORE_1);
		bb_add_byte(compiler->buffer, (unsigned char) index);
	} else if (env_contains(compiler->globals, name, name_len)) {
		int64_t index = env_get(compiler->globals, name, name_len);
		if (is_const(index)) {
			YASL_PRINT_ERROR_CONSTANT(name, line);
			handle_error(compiler);
			return;
		}
		bb_add_byte(compiler->buffer, GSTORE_8);
		bb_intbytes8(compiler->buffer, compiler->num);
		bb_intbytes8(compiler->buffer, table_search_string_int(compiler->strings, name, name_len).value.ival);
	} else {
		YASL_PRINT_ERROR_UNDECLARED_VAR(name, line);
		handle_error(compiler);
		return;
	}
}

static int contains_var_in_current_scope(const struct Compiler *const compiler, char *name, size_t name_len) {
	return in_function(compiler) ?
	       env_contains_cur_scope(compiler->params, name, name_len) :
	       compiler->stack ?
	       env_contains_cur_scope(compiler->stack, name, name_len) :
	       env_contains_cur_scope(compiler->globals, name, name_len);
}

static int contains_var(const struct Compiler *const compiler, char *name, size_t name_len) {
	return env_contains(compiler->stack, name, name_len) ||
		env_contains(compiler->params, name, name_len) ||
		env_contains_cur_scope(compiler->globals, name, name_len);
}

static void decl_var(struct Compiler *const compiler, char *name, size_t name_len, size_t line) {
	if (in_function(compiler)) {
		int64_t index = env_decl_var(compiler->params, name, name_len);
		if (index > 255) {
			YASL_PRINT_ERROR_TOO_MANY_VAR(line);
			handle_error(compiler);
		}
	} else if (compiler->stack) {
		int64_t index = env_decl_var(compiler->stack, name, name_len);
		if (index > 255) {
			YASL_PRINT_ERROR_TOO_MANY_VAR(line);
			handle_error(compiler);
		}
	} else {
		struct YASL_Object value = table_search_string_int(compiler->strings, name, name_len);
		if (value.type == Y_END) {
			YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
			table_insert_string_int(compiler->strings, name, name_len, compiler->header->count);
			bb_intbytes8(compiler->header, name_len);
			bb_append(compiler->header, (unsigned char *) name, name_len);
		}
		env_decl_var(compiler->globals, name, name_len);
		//if (index > 255) {
		//	YASL_PRINT_ERROR_TOO_MANY_VAR(line);
		//	handle_error(compiler);
		//}
	}
}

static void make_const(struct Compiler * const compiler, char *name, size_t name_len) {
	if (in_function(compiler)) env_make_const(compiler->params, name, name_len);
	else if (compiler->stack) env_make_const(compiler->stack, name, name_len);
	else env_make_const(compiler->globals, name, name_len);
}

static unsigned char *return_bytes(struct Compiler *const compiler) {
	if (compiler->status) return NULL;

	bb_rewrite_intbytes8(compiler->header, 0, compiler->header->count);
	bb_rewrite_intbytes8(compiler->header, 8, compiler->code->count + compiler->header->count + 1);   // TODO: put num globals here eventually.

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
	YASL_BYTECODE_DEBUG_LOG("%02x\n", HALT);

	fflush(stdout);
	unsigned char *bytecode = (unsigned char *)malloc(compiler->code->count + compiler->header->count + 1);    // NOT OWN
	memcpy(bytecode, compiler->header->bytes, compiler->header->count);
	memcpy(bytecode + compiler->header->count, compiler->code->bytes, compiler->code->count);
	bytecode[compiler->code->count + compiler->header->count] = HALT;
	return bytecode;
}

unsigned char *compile(struct Compiler *const compiler) {
	struct Node *node;
	gettok(&compiler->parser.lex);
	while (!peof(&compiler->parser)) {
		if (peof(&compiler->parser)) break;
		node = parse(&compiler->parser);
		eattok(&compiler->parser, T_SEMI);
		compiler->status |= compiler->parser.status;
		if (!compiler->parser.status) {
			visit(compiler, node);
			bb_append(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
			compiler->buffer->count = 0;
		}

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
			bb_append(compiler->code, compiler->buffer->bytes, compiler->buffer->count);
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
			bb_add_byte(compiler->buffer, POP);
		}
	}
}

static void visit_FunctionDecl(struct Compiler *const compiler, const struct Node *const node) {
	if (in_function(compiler)) {
		YASL_PRINT_ERROR_SYNTAX("Illegal function declaration outside global scope, in line %zd.\n",
					node->line);
		handle_error(compiler);
		return;
	}

	// start logic for function, now that we are sure it's legal to do so, and have set up.

	compiler->params = env_new(compiler->params);
	compiler->num_locals = 0;

	enter_scope(compiler);

	FOR_CHILDREN(i, child, FnDecl_get_params(node)) {
		decl_var(compiler, child->value.sval.str,
			 child->value.sval.str_len, child->line);
		if (child->nodetype == N_CONST) {
			make_const(compiler, child->value.sval.str,
				   child->value.sval.str_len);
		}
	}

	size_t old_size = compiler->buffer->count;
	// TODO: verfiy that number of params is small enough. (same for the other casts below.)
	bb_add_byte(compiler->buffer, (unsigned char)FnDecl_get_params(node)->children_len);
	size_t index = compiler->buffer->count;
	bb_add_byte(compiler->buffer, (unsigned char)compiler->params->vars->count);
	visit_Body(compiler, FnDecl_get_body(node));
	exit_scope(compiler);
	compiler->buffer->bytes[index] = (unsigned char)compiler->num_locals;

	int64_t fn_val = compiler->header->count;
	bb_append(compiler->header, compiler->buffer->bytes + old_size, compiler->buffer->count - old_size);
	compiler->buffer->count = old_size;
	bb_add_byte(compiler->header, NCONST);
	bb_add_byte(compiler->header, RET);

	// zero buffer length
	compiler->buffer->count = old_size;
	// compiler->buffer->count = 0;

	Env_t *tmp = compiler->params->parent;
	env_del_current_only(compiler->params);
	compiler->params = tmp;

	bb_add_byte(compiler->buffer, FCONST);
	bb_intbytes8(compiler->buffer, fn_val);
}

static void visit_Call(struct Compiler *const compiler, const struct Node *const node) {
	YASL_COMPILE_DEBUG_LOG("Visit Call: %s\n", node->value.sval.str);
	visit(compiler, node->children[1]);
	bb_add_byte(compiler->buffer, INIT_CALL);
	visit_Body(compiler, Call_get_params(node));
	bb_add_byte(compiler->buffer, CALL);
}

static void visit_MethodCall(struct Compiler *const compiler, const struct Node *const node) {
	YASL_COMPILE_DEBUG_LOG("Visit MethodCall: %s\n", node->value.sval.str);
	visit(compiler, node->children[1]);
	enum SpecialStrings index = get_special_string(node);
	if (index != S_UNKNOWN_STR) {
		bb_add_byte(compiler->buffer, INIT_MC_SPECIAL);
		bb_add_byte(compiler->buffer, index);
	} else {
		struct YASL_Object value = table_search_string_int(compiler->strings, node->value.sval.str, node->value.sval.str_len);
		if (value.type == Y_END) {
			YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
			table_insert_string_int(compiler->strings, node->value.sval.str, node->value.sval.str_len, compiler->header->count);
			bb_intbytes8(compiler->header, node->value.sval.str_len);
			bb_append(compiler->header, (unsigned char *) node->value.sval.str, node->value.sval.str_len);
		}

		value = table_search_string_int(compiler->strings, node->value.sval.str, node->value.sval.str_len);

		bb_add_byte(compiler->buffer, INIT_MC);
		bb_intbytes8(compiler->buffer, compiler->num);
		bb_intbytes8(compiler->buffer, value.value.ival);
	}

	visit_Body(compiler, Call_get_params(node));
	bb_add_byte(compiler->buffer, CALL);
}

static void visit_Return(struct Compiler *const compiler, const struct Node *const node) {
	YASL_COMPILE_DEBUG_LOG("Visit Return: %s\n", node->value.sval.str);
	visit(compiler, Return_get_expr(node));
	bb_add_byte(compiler->buffer, RET);
}

static void visit_Export(struct Compiler *const compiler, const struct Node *const node) {
	if (compiler->stack != NULL || compiler->params != NULL) {
		YASL_PRINT_ERROR("export statement must be at top level of module. (line %zd)\n", node->line);
		handle_error(compiler);
		return;
	}
	YASL_COMPILE_DEBUG_LOG("Visit Export: %s\n", node->value.sval.str);
	visit(compiler, Export_get_expr(node));
	bb_add_byte(compiler->buffer, EXPORT);
}

static void visit_Set(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Set_get_collection(node));
	visit(compiler, Set_get_key(node));
	visit(compiler, Set_get_value(node));
	bb_add_byte(compiler->buffer, SET);
}

static void visit_Get(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Get_get_collection(node));
	visit(compiler, Get_get_value(node));
	bb_add_byte(compiler->buffer, GET);
}

static void visit_Slice(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Slice_get_collection(node));
	visit(compiler, Slice_get_start(node));
	visit(compiler, Slice_get_end(node));
	bb_add_byte(compiler->buffer, SLICE);
}

static void visit_Block(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);
	visit(compiler, node->children[0]);
	exit_scope(compiler);
}

static inline void branch_back(struct Compiler *const compiler, int64_t index) {
	bb_add_byte(compiler->buffer, BR_8);
	bb_intbytes8(compiler->buffer, index - compiler->buffer->count - 8);
}

static void visit_ListComp(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);

	visit(compiler, node->children[1]->children[1]);

	bb_add_byte(compiler->buffer, INITFOR);

	bb_add_byte(compiler->buffer, END);

	decl_var(compiler, node->children[1]->children[0]->value.sval.str, node->children[1]->children[0]->value.sval.str_len, node->children[1]->children[0]->line);

	int64_t index_start = compiler->buffer->count;

	bb_add_byte(compiler->buffer, ITER_1);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, node->children[1]->children[0]->value.sval.str, node->children[1]->children[0]->value.sval.str_len, node->line);

	if (node->children[2]) {
		int64_t index_third;
		visit(compiler, node->children[2]);
		enter_conditional_false(compiler, &index_third);

		visit(compiler, ListComp_get_expr(node));

		exit_conditional_false(compiler, &index_third);
	} else {
		visit(compiler, ListComp_get_expr(node));
	}

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	bb_add_byte(compiler->buffer, NEWLIST);

	bb_add_byte(compiler->buffer, ENDCOMP);
	exit_scope(compiler);
}

static void visit_TableComp(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);

	visit(compiler, node->children[1]->children[1]);

	bb_add_byte(compiler->buffer, INITFOR);
	bb_add_byte(compiler->buffer, END);

	decl_var(compiler, node->children[1]->children[0]->value.sval.str, node->children[1]->children[0]->value.sval.str_len,  node->children[1]->children[0]->line);

	int64_t index_start = compiler->buffer->count;

	bb_add_byte(compiler->buffer, ITER_1);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);

	store_var(compiler, node->children[1]->children[0]->value.sval.str, node->children[1]->children[0]->value.sval.str_len, node->line);

	if (node->children[2]) {
		int64_t index_third;
		visit(compiler, node->children[2]);
		enter_conditional_false(compiler, &index_third);

		visit(compiler, TableComp_get_key_value(node)->children[0]);
		visit(compiler, TableComp_get_key_value(node)->children[1]);

		exit_conditional_false(compiler, &index_third);
	} else {
		visit(compiler, TableComp_get_key_value(node)->children[0]);
		visit(compiler, TableComp_get_key_value(node)->children[1]);
	}

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	bb_add_byte(compiler->buffer, NEWTABLE);
	bb_add_byte(compiler->buffer, ENDCOMP);

	exit_scope(compiler);
}

static void visit_ForIter(struct Compiler *const compiler, const struct Node *const node) {
	enter_scope(compiler);

	visit(compiler, node->children[0]->children[1]);

	bb_add_byte(compiler->buffer, INITFOR);

	decl_var(compiler, node->children[0]->children[0]->value.sval.str, node->children[0]->children[0]->value.sval.str_len, node->children[0]->children[0]->line);

	size_t index_start = compiler->buffer->count;
	add_checkpoint(compiler, index_start);

	bb_add_byte(compiler->buffer, ITER_1);

	add_checkpoint(compiler, compiler->buffer->count);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);


	store_var(compiler, node->children[0]->children[0]->value.sval.str, node->children[0]->children[0]->value.sval.str_len, node->line);

	visit(compiler, ForIter_get_body(node));

	branch_back(compiler, index_start);

	exit_conditional_false(compiler, &index_second);

	bb_add_byte(compiler->buffer, ENDFOR);
	exit_scope(compiler);

	rm_checkpoint(compiler);
	rm_checkpoint(compiler);
}

static void visit_While(struct Compiler *const compiler, const struct Node *const node) {
	int64_t index_start = compiler->buffer->count;

	if (node->children[2] != NULL) {
		bb_add_byte(compiler->buffer, BR_8);
		size_t index = compiler->buffer->count;
		bb_intbytes8(compiler->buffer, 0);
		index_start = compiler->buffer->count;
		visit(compiler, node->children[2]);
		bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count - index - 8);
	}

	add_checkpoint(compiler, index_start);

	visit(compiler, While_get_cond(node));

	add_checkpoint(compiler, compiler->buffer->count);

	int64_t index_second;
	enter_conditional_false(compiler, &index_second);
	enter_scope(compiler);

	visit(compiler, While_get_body(node));

	branch_back(compiler, index_start);

	exit_scope(compiler);
	exit_conditional_false(compiler, &index_second);

	rm_checkpoint(compiler);
	rm_checkpoint(compiler);
}

static void visit_Break(struct Compiler *const compiler, const struct Node *const node) {
	if (compiler->checkpoints_count == 0) {
		YASL_PRINT_ERROR_SYNTAX("break outside of loop (line %zd).\n", node->line);
		handle_error(compiler);
		return;
	}
	bb_add_byte(compiler->buffer, BCONST_F);
	branch_back(compiler, break_checkpoint(compiler));
}

static void visit_Continue(struct Compiler *const compiler, const struct Node *const node) {
	if (compiler->checkpoints_count == 0) {
		YASL_PRINT_ERROR_SYNTAX("continue outside of loop (line %zd).\n", node->line);
		handle_error(compiler);
		return;
	}
	branch_back(compiler, continue_checkpoint(compiler));
}

static void visit_If(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, node->children[0]);

	int64_t index_then;
	enter_conditional_false(compiler, &index_then);
	enter_scope(compiler);

	visit(compiler, node->children[1]);

	int64_t index_else = 0;

	if (node->children[2] != NULL) {
		bb_add_byte(compiler->buffer, BR_8);
		index_else = compiler->buffer->count;
		bb_intbytes8(compiler->buffer, 0);
	}

	exit_scope(compiler);
	exit_conditional_false(compiler, &index_then);

	if (node->children[2] != NULL) {
		enter_scope(compiler);
		visit(compiler, node->children[2]);
		exit_scope(compiler);
		bb_rewrite_intbytes8(compiler->buffer, index_else, compiler->buffer->count - index_else - 8);
	}
}

static void visit_Print(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, Print_get_expr(node));
	bb_add_byte(compiler->buffer, PRINT);
}

static void declare_with_let_or_const(struct Compiler *const compiler, const struct Node *const node) {
	if (contains_var_in_current_scope(compiler, node->value.sval.str, node->value.sval.str_len)) {
		YASL_PRINT_ERROR_SYNTAX("Illegal redeclaration of %s (line %zd).\n", node->value.sval.str, node->line);
		handle_error(compiler);
		return;
	}

	decl_var(compiler, node->value.sval.str, node->value.sval.str_len, node->line);

	if (Let_get_expr(node) != NULL) visit(compiler, Let_get_expr(node));
	else bb_add_byte(compiler->buffer, NCONST);

	store_var(compiler, node->value.sval.str, node->value.sval.str_len, node->line);
}

static void visit_Let(struct Compiler *const compiler, const struct Node *const node) {
	declare_with_let_or_const(compiler, node);
}

static void visit_Const(struct Compiler *const compiler, const struct Node *const node) {
	declare_with_let_or_const(compiler, node);
	make_const(compiler, node->value.sval.str, node->value.sval.str_len);
}

static void visit_TriOp(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, node->children[0]);

	int64_t index_l;
	enter_conditional_false(compiler, &index_l);

	visit(compiler, node->children[1]);

	bb_add_byte(compiler->buffer, BR_8);
	size_t index_r = compiler->buffer->count;
	bb_intbytes8(compiler->buffer, 0);

	exit_conditional_false(compiler, &index_l);

	visit(compiler, node->children[2]);
	bb_rewrite_intbytes8(compiler->buffer, index_r, compiler->buffer->count - index_r - 8);
}

static void visit_BinOp(struct Compiler *const compiler, const struct Node *const node) {
	// complicated bin ops are handled on their own.
	if (node->type == T_DQMARK) {     // ?? operator
		visit(compiler, node->children[0]);
		bb_add_byte(compiler->buffer, DUP);
		bb_add_byte(compiler->buffer, BRN_8);
		size_t index = compiler->buffer->count;
		bb_intbytes8(compiler->buffer, 0);
		bb_add_byte(compiler->buffer, POP);
		visit(compiler, node->children[1]);
		bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count - index - 8);
		return;
	} else if (node->type == T_DBAR) {  // or operator
		visit(compiler, node->children[0]);
		bb_add_byte(compiler->buffer, DUP);
		bb_add_byte(compiler->buffer, BRT_8);
		size_t index = compiler->buffer->count;
		bb_intbytes8(compiler->buffer, 0);
		bb_add_byte(compiler->buffer, POP);
		visit(compiler, node->children[1]);
		bb_rewrite_intbytes8(compiler->buffer, index, compiler->buffer->count - index - 8);
		return;
	} else if (node->type == T_DAMP) {   // and operator
		visit(compiler, node->children[0]);
		bb_add_byte(compiler->buffer, DUP);

		int64_t index;
		enter_conditional_false(compiler, &index);

		bb_add_byte(compiler->buffer, POP);
		visit(compiler, node->children[1]);
		exit_conditional_false(compiler, &index);
		return;
	}
	// all other operators follow the same pattern of visiting one child then the other.
	visit(compiler, node->children[0]);
	visit(compiler, node->children[1]);
	switch (node->type) {
	case T_BAR:
		bb_add_byte(compiler->buffer, BOR);
		break;
	case T_CARET:
		bb_add_byte(compiler->buffer, BXOR);
		break;
	case T_AMP:
		bb_add_byte(compiler->buffer, BAND);
		break;
	case T_AMPCARET:
		bb_add_byte(compiler->buffer, BANDNOT);
		break;
	case T_DEQ:
		bb_add_byte(compiler->buffer, EQ);
		break;
	case T_TEQ:
		bb_add_byte(compiler->buffer, ID);
		break;
	case T_BANGEQ:
		bb_add_byte(compiler->buffer, EQ);
		bb_add_byte(compiler->buffer, NOT);
		break;
	case T_BANGDEQ:
		bb_add_byte(compiler->buffer, ID);
		bb_add_byte(compiler->buffer, NOT);
		break;
	case T_GT:
		bb_add_byte(compiler->buffer, GT);
		break;
	case T_GTEQ:
		bb_add_byte(compiler->buffer, GE);
		break;
	case T_LT:
		bb_add_byte(compiler->buffer, GE);
		bb_add_byte(compiler->buffer, NOT);
		break;
	case T_LTEQ:
		bb_add_byte(compiler->buffer, GT);
		bb_add_byte(compiler->buffer, NOT);
		break;
	case T_TILDE:
		bb_add_byte(compiler->buffer, CNCT);
		break;
	case T_DGT:
		bb_add_byte(compiler->buffer, BSR);
		break;
	case T_DLT:
		bb_add_byte(compiler->buffer, BSL);
		break;
	case T_PLUS:
		bb_add_byte(compiler->buffer, ADD);
		break;
	case T_MINUS:
		bb_add_byte(compiler->buffer, SUB);
		break;
	case T_STAR:
		bb_add_byte(compiler->buffer, MUL);
		break;
	case T_SLASH:
		bb_add_byte(compiler->buffer, FDIV);
		break;
	case T_DSLASH:
		bb_add_byte(compiler->buffer, IDIV);
		break;
	case T_MOD:
		bb_add_byte(compiler->buffer, MOD);
		break;
	case T_DSTAR:
		bb_add_byte(compiler->buffer, EXP);
		break;
	default:
		puts("error in visit_BinOp");
		exit(EXIT_FAILURE);
	}
}

static void visit_UnOp(struct Compiler *const compiler, const struct Node *const node) {
	visit(compiler, UnOp_get_expr(node));
	switch (node->type) {
	case T_PLUS:
		bb_add_byte(compiler->buffer, POS);
		break;
	case T_MINUS:
		bb_add_byte(compiler->buffer, NEG);
		break;
	case T_BANG:
		bb_add_byte(compiler->buffer, NOT);
		break;
	case T_CARET:
		bb_add_byte(compiler->buffer, BNOT);
		break;
	case T_LEN:
		bb_add_byte(compiler->buffer, LEN);
		break;
	default:
		puts("error in visit_UnOp");
		exit(EXIT_FAILURE);
	}
}

static void visit_Assign(struct Compiler *const compiler, const struct Node *const node) {
	if (!contains_var(compiler, node->value.sval.str, node->value.sval.str_len)) {
		YASL_PRINT_ERROR_UNDECLARED_VAR(node->value.sval.str, node->line);
		handle_error(compiler);
		return;
	}
	visit(compiler, Assign_get_expr(node));
	store_var(compiler, node->value.sval.str, node->value.sval.str_len, node->line);
	// load_var(compiler, node->value.sval.str, node->value.sval.str_len, node->line);
}

static void visit_Var(struct Compiler *const compiler, const struct Node *const node) {
	load_var(compiler, node->value.sval.str, node->value.sval.str_len, node->line);
}

static void visit_Undef(struct Compiler *const compiler, const struct Node *const node) {
	bb_add_byte(compiler->buffer, NCONST);
}

static void visit_Float(struct Compiler *const compiler, const struct Node *const node) {
	double val = node->value.dval;
	if (val == 0.0) {
		bb_add_byte(compiler->buffer, DCONST_0);
	} else if (val == 1.0) {
		bb_add_byte(compiler->buffer, DCONST_1);
	} else if (val == 2.0) {
		bb_add_byte(compiler->buffer, DCONST_2);
	} else if (val != val) {
		bb_add_byte(compiler->buffer, DCONST_N);
	} else if (isinf(val)) {
		bb_add_byte(compiler->buffer, DCONST_I);
	} else {
		bb_add_byte(compiler->buffer, DCONST);
		bb_floatbytes8(compiler->buffer, val);
	}
}

static void visit_Integer(struct Compiler *const compiler, const struct Node *const node) {
	YASL_COMPILE_DEBUG_LOG("int64: %" PRId64 "\n", node->value.ival);
	yasl_int val = node->value.ival;
	switch (val) {
	case -1:
		bb_add_byte(compiler->buffer, ICONST_M1);
		break;
	case 0:
		bb_add_byte(compiler->buffer, ICONST_0);
		break;
	case 1:
		bb_add_byte(compiler->buffer, ICONST_1);
		break;
	case 2:
		bb_add_byte(compiler->buffer, ICONST_2);
		break;
	case 3:
		bb_add_byte(compiler->buffer, ICONST_3);
		break;
	case 4:
		bb_add_byte(compiler->buffer, ICONST_4);
		break;
	case 5:
		bb_add_byte(compiler->buffer, ICONST_5);
		break;
	default:
		bb_add_byte(compiler->buffer, ICONST);
		bb_intbytes8(compiler->buffer, val);
		break;
	}
}

static void visit_Boolean(struct Compiler *const compiler, const struct Node *const node) {
	bb_add_byte(compiler->buffer, node->value.ival ? BCONST_T : BCONST_F);
}

static void visit_String(struct Compiler *const compiler, const struct Node *const node) {
	struct YASL_Object value = table_search_string_int(compiler->strings, node->value.sval.str, node->value.sval.str_len);
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
		table_insert_string_int(compiler->strings, node->value.sval.str, node->value.sval.str_len, compiler->header->count);
		bb_intbytes8(compiler->header, node->value.sval.str_len);
		bb_append(compiler->header, (unsigned char *) node->value.sval.str, node->value.sval.str_len);
	}

	value = table_search_string_int(compiler->strings, node->value.sval.str, node->value.sval.str_len);

	enum SpecialStrings index = get_special_string(node);
	if (index != S_UNKNOWN_STR) {
		bb_add_byte(compiler->buffer, NEWSPECIALSTR);
		bb_add_byte(compiler->buffer, index);
	} else {
		bb_add_byte(compiler->buffer, NEWSTR);
		bb_intbytes8(compiler->buffer, compiler->num);
		bb_intbytes8(compiler->buffer, value.value.ival);
	}
}

static void make_new_collection(struct Compiler *const compiler, const struct Node *const node, enum Opcode type) {
	bb_add_byte(compiler->buffer, END);
	visit_Body(compiler, node);
	bb_add_byte(compiler->buffer, type);
}

static void visit_List(struct Compiler *const compiler, const struct Node *const node) {
	make_new_collection(compiler, List_get_values(node), NEWLIST);
}

static void visit_Table(struct Compiler *const compiler, const struct Node *const node) {
	make_new_collection(compiler, Table_get_values(node), NEWTABLE);
}

// NOTE: _MUST_ keep this synced with the enum in ast.h, and the jumptable in middleend.c
static void (*jumptable[])(struct Compiler *const, const struct Node *const) = {
	&visit_ExprStmt,
	&visit_Block,
	&visit_Body,
	&visit_FunctionDecl,
	&visit_Return,
	&visit_Export,
	&visit_Call,
	&visit_MethodCall,
	&visit_Set,
	&visit_Get,
	&visit_Slice,
	NULL,
	&visit_ListComp,
	&visit_TableComp,
	&visit_ForIter,
	&visit_While,
	&visit_Break,
	&visit_Continue,
	&visit_If,
	&visit_Print,
	&visit_Let,
	&visit_Const,
	&visit_TriOp,
	&visit_BinOp,
	&visit_UnOp,
	&visit_Assign,
	&visit_Var,
	&visit_Undef,
	&visit_Float,
	&visit_Integer,
	&visit_Boolean,
	&visit_String,
	&visit_List,
	&visit_Table
};

static void visit(struct Compiler *const compiler, const struct Node *const node) {
	jumptable[node->nodetype](compiler, node);
}
