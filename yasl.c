#include "yasl.h"

#include <stdarg.h>
#include <interpreter/YASL_Object.h>

#include "interpreter/table_methods.h"
#include "interpreter/userdata.h"
#include "yasl_state.h"
#include "compiler/compiler.h"
#include "interpreter/VM.h"
#include "compiler/lexinput.h"

struct YASL_State *YASL_newstate_num(const char *filename, size_t num) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return NULL;  // Can't open file.
	}

	struct YASL_State *S = (struct YASL_State *)malloc(sizeof(struct YASL_State));

	fseek(fp, 0, SEEK_SET);

	struct LEXINPUT *lp = lexinput_new_file(fp);
	struct Compiler tcomp = NEW_COMPILER(lp);
	S->compiler = tcomp;
	S->compiler.num = num;
	S->compiler.header->count = 24;

	vm_init((struct VM *)S, NULL, -1, num + 1);
	return S;
}

struct YASL_State *YASL_newstate(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return NULL;  // Can't open file.
	}

	struct YASL_State *S = (struct YASL_State *) malloc(sizeof(struct YASL_State));

	fseek(fp, 0, SEEK_SET);

	struct LEXINPUT *lp = lexinput_new_file(fp);
	struct Compiler tcomp = NEW_COMPILER(lp);
	S->compiler = tcomp;
	S->compiler.header->count = 24;
	S->compiler.num = 0;

	vm_init((struct VM *) S, NULL, -1, 1);

	YASL_declglobal(S, "__VERSION__");
	YASL_pushlit(S, YASL_VERSION);
	YASL_setglobal(S, "__VERSION__");

	return S;
}

void YASL_setprintout_tostr(struct YASL_State *S) {
	S->vm.out.print = &io_print_string;
}

void YASL_setprinterr_tostr(struct YASL_State *S) {
	S->compiler.parser.lex.err.print = &io_print_string;
	S->vm.err.print = &io_print_string;
}

void YASL_loadprintout(struct YASL_State *S) {
	YASL_pushlstr(S, S->vm.out.string, S->vm.out.len);
}

void YASL_loadprinterr(struct YASL_State *S) {
	if (S->compiler.status != YASL_SUCCESS) {
		YASL_pushlstr(S, S->compiler.parser.lex.err.string, S->compiler.parser.lex.err.len);
	} else {
		YASL_pushlstr(S, S->vm.err.string, S->vm.err.len);
	}
}

int YASL_resetstate(struct YASL_State *S, const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return YASL_ERROR;  // Can't open file.
	}

	fseek(fp, 0, SEEK_SET);

	S->compiler.status = YASL_SUCCESS;
	S->compiler.parser.status = YASL_SUCCESS;
	lex_cleanup(&S->compiler.parser.lex);

	S->compiler.parser.lex = NEW_LEXER(lexinput_new_file(fp));
	S->compiler.code->count = 0;
	S->compiler.buffer->count = 0;

	return YASL_SUCCESS;
}

struct YASL_State *YASL_newstate_bb(const char *buf, size_t len) {
	struct YASL_State *S = (struct YASL_State *) malloc(sizeof(struct YASL_State));

	struct LEXINPUT *lp = lexinput_new_bb(buf, len);
	struct Compiler tcomp = NEW_COMPILER(lp);
	S->compiler = tcomp;
	S->compiler.header->count = 24;
	S->compiler.num = 0;

	vm_init((struct VM *) S, NULL, -1, 1);
	return S;
}

int YASL_resetstate_bb(struct YASL_State *S, const char *buf, size_t len) {
	S->compiler.status = YASL_SUCCESS;
	S->compiler.parser.status = YASL_SUCCESS;
	lex_cleanup(&S->compiler.parser.lex);
	S->compiler.parser.lex = NEW_LEXER(lexinput_new_bb(buf, len));
	S->compiler.code->count = 0;
	S->compiler.buffer->count = 0;
	if (S->vm.code)	free(S->vm.code);
	S->vm.code = NULL;

	return YASL_SUCCESS;
}

int YASL_delstate(struct YASL_State *S) {
	if (!S) return YASL_SUCCESS;

	compiler_cleanup(&S->compiler);
	vm_cleanup((struct VM *) S);
	free(S);
	return YASL_SUCCESS;
}

int YASL_execute_REPL(struct YASL_State *S) {
	unsigned char *bc = compile_REPL(&S->compiler);
	if (!bc) return S->compiler.status;

	int64_t entry_point = *((int64_t*)bc);
	// TODO: use this in VM.
	// int64_t num_globals = *((int64_t*)bc+1);

	S->vm.pc = bc + entry_point;
	S->vm.code = bc;
	S->vm.headers[S->compiler.num] = bc;

	return vm_run((struct VM *)S);  // TODO: error handling for runtime errors.
}

int YASL_compile(struct YASL_State *S) {
	compile(&S->compiler);
	return S->compiler.status;
}

int YASL_execute(struct YASL_State *S) {
	unsigned char *bc = compile(&S->compiler);
	if (!bc) return S->compiler.status;

	int64_t entry_point = *((int64_t *) bc);
	// TODO: use this in VM.
	// int64_t num_globals = *((int64_t*)bc+1);

	S->vm.pc = bc + entry_point;
	S->vm.code = bc;
	S->vm.headers[S->compiler.num] = bc;

	int result = vm_run((struct VM *) S);  // TODO: error handling for runtime errors.

	return result;
}

int YASL_declglobal(struct YASL_State *S, const char *name) {
	compiler_intern_string(&S->compiler, name, strlen(name));
	scope_decl_var(S->compiler.globals, name);
	return YASL_SUCCESS;
}

static inline int is_const(int64_t value) {
	const uint64_t MASK = 0x8000000000000000;
	return (MASK & value) != 0;
}

int YASL_setglobal(struct YASL_State *S, const char *name) {
	if (!scope_contains(S->compiler.globals, name)) return YASL_ERROR;

	int64_t index = scope_get(S->compiler.globals, name);
	if (is_const(index)) return YASL_ERROR;

	struct YASL_String *string = YASL_String_new_sized(strlen(name), name);
	YASL_Table_insert_fast(S->vm.globals, YASL_STR(string), vm_peek((struct VM *) S));
	YASL_pop(S);

	return YASL_SUCCESS;
}

int YASL_loadglobal(struct YASL_State *S, const char *name) {
	struct YASL_String *string = YASL_String_new_sized(strlen(name), name);
	struct YASL_Object global = YASL_Table_search(S->vm.globals, YASL_STR(string));
	str_del(string);
	if (global.type == Y_END) {
		return YASL_ERROR;
	}
	vm_push(&S->vm, global);
	return YASL_SUCCESS;
}

int YASL_registermt(struct YASL_State *S, const char *name) {
	struct YASL_String *string = YASL_String_new_sized(strlen(name), name);
	YASL_Table_insert_fast(S->vm.metatables, YASL_STR(string), vm_peek((struct VM *) S));
	YASL_pop(S);

	return YASL_SUCCESS;
}

int YASL_loadmt(struct YASL_State *S, const char *name) {
	struct YASL_String *string = YASL_String_new_sized(strlen(name), name);
	struct YASL_Object mt = YASL_Table_search(S->vm.metatables, YASL_STR(string));
	str_del(string);
	if (mt.type == Y_END) {
		return YASL_ERROR;
	}
	vm_push(&S->vm, mt);
	return YASL_SUCCESS;
}

int YASL_setmt(struct YASL_State *S) {
	if (!YASL_istable(S) && !YASL_isundef(S)) {
		return YASL_TYPE_ERROR;
	}

	struct RC_UserData *mt = YASL_istable(S) ? YASL_GETUSERDATA(vm_pop(&S->vm)) : NULL;

	if (!vm_isuserdata(&S->vm) && !vm_istable(&S->vm) && !vm_islist(&S->vm)) {
		return YASL_TYPE_ERROR;
	}

	ud_setmt(vm_peek(&S->vm).value.uval, mt);

	return YASL_SUCCESS;
}

void YASL_print_err(struct YASL_State *S, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vvm_print_err(&S->vm, fmt, args);
	va_end(args);
}

void YASL_throw_err(struct YASL_State *S, int error) {
	vm_throw_err(&S->vm, error);
}

int YASL_peektype(struct YASL_State *S) {
	return vm_peek(&S->vm).type;
}

int YASL_peekntype(struct YASL_State *S, unsigned n) {
	return vm_peek(&S->vm, S->vm.fp + 1 + n).type;
}

const char *YASL_peektypename(struct YASL_State *S) {
	return obj_typename(vm_peek_p(&S->vm));
}

const char *YASL_peektypestr(struct YASL_State *S) {
	return YASL_peektypename(S);
}

const char *YASL_peekntypename(struct YASL_State *S, unsigned n) {
	return obj_typename(vm_peek_p(&S->vm, S->vm.fp + 1 + n));
}

const char *YASL_peekntypestr(struct YASL_State *S, unsigned n) {
	return YASL_peekntypename(S, n);
}

void YASL_pushundef(struct YASL_State *S) {
	vm_push((struct VM *) S, YASL_UNDEF());
}

void YASL_pushfloat(struct YASL_State *S, yasl_float value) {
	vm_pushfloat((struct VM *) S, value);
}

void YASL_pushint(struct YASL_State *S, yasl_int value) {
	vm_pushint(&S->vm, value);
}

void YASL_pushbool(struct YASL_State *S, bool value) {
	vm_pushbool(&S->vm, value);
}

void YASL_pushliteralstring(struct YASL_State *S, char *value) {
	vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized(strlen(value), value)));
}

void YASL_pushcstring(struct YASL_State *S, char *value) {
	vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized(strlen(value), value)));
}

void YASL_pushuserdata(struct YASL_State *S, void *data, const char *tag, void (*destructor)(void *)) {
	vm_push(&S->vm, YASL_USERDATA(ud_new(data, tag, NULL, destructor)));
}

void YASL_pushuserptr(struct YASL_State *S, void *userpointer) {
	vm_push((struct VM *) S, YASL_USERPTR(userpointer));
}

void YASL_pushszstring(struct YASL_State *S, const char *value) {
	vm_pushstr((struct VM *) S, YASL_String_new_sized_heap(0, strlen(value), value));
}

void YASL_pushlitszstring(struct YASL_State *S, const char *value) {
	YASL_pushlit(S, value);
}

void YASL_pushlit(struct YASL_State *S, const char *value) {
	vm_pushstr((struct VM *) S, YASL_String_new_sized(strlen(value), value));
}

void YASL_pushzstr(struct YASL_State *S, const char *value) {
	YASL_pushlstr(S, value, strlen(value));
}

void YASL_pushlstr(struct YASL_State *S, const char *value, size_t len) {
	char *buffer = (char *)malloc(len);
	memcpy(buffer, value, len);
	vm_pushstr((struct VM *)S, YASL_String_new_sized_heap(0, len, buffer));
}

void YASL_pushstring(struct YASL_State *S, const char *value, const size_t size) {
	vm_pushstr((struct VM *) S, YASL_String_new_sized_heap(0, size, value));
}

void YASL_pushlitstring(struct YASL_State *S, const char *value, const size_t size) {
	vm_pushstr((struct VM *) S, YASL_String_new_sized(size, value));
}

void YASL_pushcfunction(struct YASL_State *S, YASL_cfn value, int num_args) {
	vm_push((struct VM *) S, YASL_CFN(value, num_args));
}

void YASL_pushtable(struct YASL_State *S) {
	struct RC_UserData *table = rcht_new();
	ud_setmt(table, S->vm.builtins_htable[Y_TABLE]);
	vm_push(&S->vm, YASL_TABLE(table));
}

void YASL_pushlist(struct YASL_State *S) {
	struct RC_UserData *list = rcls_new();
	ud_setmt(list, S->vm.builtins_htable[Y_LIST]);
	vm_push(&S->vm, YASL_LIST(list));
}

yasl_int YASL_peekvargscount(struct YASL_State *S) {
	struct VM *vm = (struct VM *)S;
	yasl_int num_args = vm_peek(vm, vm->fp).value.cval->num_args;

	return vm_peekint(vm, vm->fp + 1 + ~num_args);
}

void YASL_pop(struct YASL_State *S) {
	YASL_ASSERT(S->vm.sp >= 0, "Cannot pop from empty stack.");
	vm_pop(&S->vm);
}

int YASL_duptop(struct YASL_State *S) {
	YASL_ASSERT(S->vm.sp >= 0, "Cannot duplicate top of empty stack.");
	vm_push(&S->vm, vm_peek(&S->vm));
	return YASL_SUCCESS;
}

bool YASL_tablenext(struct YASL_State *S) {
	struct YASL_Object key = vm_pop(&S->vm);
	if (!YASL_istable(S)) {
		return false;
	}

	struct YASL_Table *table = vm_peektable(&S->vm);

	size_t index = obj_isundef(&key) ? 0 : YASL_Table_getindex(table, key) + 1;

	while (table->size > index &&
		(table->items[index].key.type == Y_END || table->items[index].key.type == Y_UNDEF)) {
		index++;
	}

	if (table->size <= index) {
		return false;
	}

	vm_push(&S->vm, table->items[index].key);
	vm_push(&S->vm, table->items[index].value);
	return true;
}

int YASL_tableset(struct YASL_State *S) {
	struct YASL_Object value = vm_pop(&S->vm);
	struct YASL_Object key = vm_pop(&S->vm);
	struct YASL_Object table = vm_peek(&S->vm);

	if (!obj_istable(&table))
		return YASL_TYPE_ERROR;
	if (!YASL_Table_insert(YASL_GETTABLE(table), key, value)) {
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

void list___get_helper(struct YASL_State *S, struct YASL_List *ls, yasl_int index);

void vm_len_unop(struct VM *const vm);

void YASL_len(struct YASL_State *S) {
	vm_len_unop(&S->vm);
}

int YASL_listget(struct YASL_State *S, yasl_int n) {
	if (!YASL_islist(S)) {
		return YASL_TYPE_ERROR;
	}

	struct YASL_List *ls = vm_peeklist(&S->vm);
	list___get_helper(S, ls, n);

	return YASL_SUCCESS;
}

int YASL_listpush(struct YASL_State *S) {
	struct YASL_Object value = vm_pop(&S->vm);
	if (!YASL_islist(S)) {
		return YASL_TYPE_ERROR;
	}

	struct YASL_List *list = vm_peeklist(&S->vm);

	YASL_List_append(list, value);

	return YASL_SUCCESS;
}

int YASL_functioncall(struct YASL_State *S, int n) {
	struct VM *vm = &S->vm;
	vm_INIT_CALL_offset(vm, vm->sp - n, -1);

	const int old_sp = vm->sp;
	vm_CALL(vm);

	return old_sp - vm->sp - 1;
}

bool YASL_isundef(struct YASL_State *S) {
	return vm_isundef(&S->vm);
}

bool YASL_isnundef(struct YASL_State *S, unsigned n) {
	return vm_isundef(&S->vm, S->vm.fp + 1 + n);
}

bool YASL_isbool(struct YASL_State *S) {
	return vm_isbool(&S->vm);
}

bool YASL_isnbool(struct YASL_State *S, unsigned n) {
	return vm_isbool(&S->vm, S->vm.fp + 1 + n);
}

bool YASL_isfloat(struct YASL_State *S) {
	return vm_isfloat(&S->vm);
}

bool YASL_isnfloat(struct YASL_State *S, unsigned n) {
	return vm_isfloat(&S->vm, S->vm.fp + 1 + n);
}

bool YASL_isint(struct YASL_State *S) {
	return vm_isint(&S->vm);
}

bool YASL_isnint(struct YASL_State *S, unsigned n) {
	return vm_isint(&S->vm, S->vm.fp + 1 + n);
}

bool YASL_isnuserdata(struct YASL_State *S, const char *tag, unsigned n) {
	return vm_isuserdata(&S->vm, S->vm.fp + 1 + n) &&
	       YASL_GETUSERDATA(vm_peek(&S->vm, S->vm.fp + 1 + n))->tag == tag;
}

bool YASL_isstr(struct YASL_State *S) {
	return vm_isstr(&S->vm);
}

bool YASL_isnstr(struct YASL_State *S, unsigned n) {
	return vm_isstr(&S->vm, S->vm.fp + 1 + n);
}

bool YASL_islist(struct YASL_State *S) {
	return vm_islist(&S->vm);
}

bool YASL_isnlist(struct YASL_State *S, unsigned n) {
	return vm_islist(&S->vm, S->vm.fp + 1 + n);
}

bool YASL_istable(struct YASL_State *S) {
	return vm_istable(&S->vm);
}

bool YASL_isntable(struct YASL_State *S, unsigned n) {
	return vm_istable(&S->vm, S->vm.fp + 1 + n);
}

bool YASL_isuserdata(struct YASL_State *S, const char *tag) {
	return vm_isuserdata(&S->vm) && YASL_GETUSERDATA(vm_peek(&S->vm))->tag == tag;
}

bool YASL_peekbool(struct YASL_State *S) {
	if (YASL_isbool(S)) {
		return vm_peekbool(&S->vm);
	}
	return false;
}

bool YASL_peeknbool(struct YASL_State *S, unsigned n) {
	if (YASL_isnbool(S, n)) {
		return vm_peekbool(&S->vm, S->vm.fp + 1 + n);
	}

	return false;
}

bool YASL_popbool(struct YASL_State *S) {
	if (YASL_isbool(S)) {
		return vm_popbool(&S->vm);
	}
	return false;
}

yasl_float YASL_peekfloat(struct YASL_State *S) {
	if (YASL_isfloat(S)) {
		return vm_peekfloat(&S->vm);
	}
	return 0.0;
}

yasl_float YASL_peeknfloat(struct YASL_State *S, unsigned n) {
	if (YASL_isnfloat(S, n)) {
		return vm_peekfloat(&S->vm, S->vm.fp + 1 + n);
	}

	return 0.0;
}

yasl_float YASL_popfloat(struct YASL_State *S) {
	if (YASL_isfloat(S)) {
		return vm_popfloat(&S->vm);
	}
	return 0.0;
}

yasl_int YASL_peekint(struct YASL_State *S) {
	if (YASL_isint(S)) {
		return vm_peekint(&S->vm);
	}
	return 0;
}

yasl_int YASL_peeknint(struct YASL_State *S, unsigned n) {
	if (YASL_isnint(S, n)) {
		return vm_peekint(&S->vm, S->vm.fp + 1 + n);
	}

	return 0;
}

yasl_int YASL_popint(struct YASL_State *S) {
	if (YASL_isint(S)) {
		return vm_popint(&S->vm);
	}
	return 0;
}

char *YASL_peekcstr(struct YASL_State *S) {
	if (!YASL_isstr(S)) return NULL;

	struct YASL_Object obj = vm_peek(&S->vm);
	char *tmp = (char *) malloc(YASL_String_len(obj.value.sval) + 1);

	memcpy(tmp, YASL_String_chars(obj.value.sval), YASL_String_len(obj.value.sval));
	tmp[YASL_String_len(obj.value.sval)] = '\0';

	return tmp;
}

char *YASL_popcstr(struct YASL_State *S) {
	char *tmp = YASL_peekcstr(S);
	YASL_pop(S);
	return tmp;
}

void *YASL_peeknuserdata(struct YASL_State *S, unsigned n) {
	return YASL_GETUSERDATA(vm_peek(&S->vm, S->vm.fp + 1 + n))->data;
}

void *YASL_popuserdata(struct YASL_State *S) {
	return YASL_GETUSERDATA(vm_pop(&S->vm))->data;
}

void *YASL_popuserptr(struct YASL_State *S) {
	return YASL_GETUSERPTR(vm_pop(&S->vm));
}
