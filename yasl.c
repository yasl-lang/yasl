#include <interpreter/table_methods.h>
#include <interpreter/YASL_Object.h>
#include <data-structures/YASL_String.h>
#include <interpreter/userdata.h>
#include <data-structures/YASL_ByteBuffer.h>
#include "yasl.h"
#include "yasl_state.h"
#include "compiler/compiler.h"
#include "interpreter/VM.h"
#include "compiler/lexinput.h"

struct YASL_State *YASL_newstate_num(char *filename, size_t num) {
	struct YASL_State *S = (struct YASL_State *)malloc(sizeof(struct YASL_State));

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return NULL;  // Can't open file.
	}

	fseek(fp, 0, SEEK_SET);

	struct LEXINPUT *lp = lexinput_new_file(fp);
	struct Compiler tcomp = NEW_COMPILER(lp);
	S->compiler = tcomp;
	S->compiler.num = num;
	S->compiler.header->count = 16;

	vm_init((struct VM *)S, NULL, -1, num + 1);
	return S;
}

struct YASL_State *YASL_newstate(const char *filename) {
	struct YASL_State *S = (struct YASL_State *) malloc(sizeof(struct YASL_State));

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return NULL;  // Can't open file.
	}

	fseek(fp, 0, SEEK_SET);

	struct LEXINPUT *lp = lexinput_new_file(fp);
	struct Compiler tcomp = NEW_COMPILER(lp);
	S->compiler = tcomp;
	S->compiler.header->count = 16;
	S->compiler.num = 0;

	vm_init((struct VM *) S, NULL, -1, 1);
	return S;
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
	S->compiler.header->count = 16;
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

	return vm_run((struct VM *) S);  // TODO: error handling for runtime errors.
}

int YASL_declglobal(struct YASL_State *S, const char *name) {
	struct YASL_Object value = YASL_Table_search_string_int(S->compiler.strings, name, strlen(name));
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
		YASL_Table_insert_string_int(S->compiler.strings, name, strlen(name), S->compiler.header->count);
		YASL_ByteBuffer_add_int(S->compiler.header, strlen(name));
		YASL_ByteBuffer_extend(S->compiler.header, (unsigned char *) name, strlen(name));
	}
	/* int64_t index =*/ env_decl_var(S->compiler.globals, name);
	return YASL_SUCCESS;
}

static inline int is_const(int64_t value) {
	const uint64_t MASK = 0x8000000000000000;
	return (MASK & value) != 0;
}

int YASL_setglobal(struct YASL_State *S, const char *name) {

	if (!env_contains(S->compiler.globals, name)) return YASL_ERROR;

	int64_t index = env_get(S->compiler.globals, name);
	if (is_const(index)) return YASL_ERROR;

	struct YASL_String *string = YASL_String_new_sized(strlen(name), name);
	YASL_Table_insert(S->vm.globals[0], YASL_STR(string), vm_peek((struct VM *) S));
	S->vm.sp--;

	return YASL_SUCCESS;
}

int YASL_loadglobal(struct YASL_State *S, const char *name) {
	struct YASL_String *string = YASL_String_new_sized(strlen(name), name);
	struct YASL_Object global = YASL_Table_search(S->vm.globals[0], YASL_STR(string));
	str_del(string);
	if (global.type == Y_END) {
		return YASL_ERROR;
	}
	vm_push(&S->vm, global);
	return YASL_SUCCESS;
}

int YASL_top_peektype(struct YASL_State *S) {
	return vm_peek(&S->vm).type;
}

int YASL_pushundef(struct YASL_State *S) {
	vm_push((struct VM *) S, YASL_UNDEF());
	return YASL_SUCCESS;
}

int YASL_pushfloat(struct YASL_State *S, yasl_float value) {
	vm_pushfloat((struct VM *) S, value);
	return YASL_SUCCESS;
}

int YASL_pushinteger(struct YASL_State *S, int64_t value) {
	vm_push((struct VM *) S, YASL_INT(value));
	return YASL_SUCCESS;
}

int YASL_pushboolean(struct YASL_State *S, int value) {
	vm_push((struct VM *) S, YASL_BOOL(value));
	return YASL_SUCCESS;
}

int YASL_pushliteralstring(struct YASL_State *S, char *value) {
	vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized(strlen(value), value)));
	return YASL_SUCCESS;
}

int YASL_pushcstring(struct YASL_State *S, char *value) {
	vm_push((struct VM *) S, YASL_STR(YASL_String_new_sized(strlen(value), value)));
	return YASL_SUCCESS;
}

int YASL_pushuserdata(struct YASL_State *S, void *data, int tag, struct YASL_Table *mt, void (*destructor)(void *)) {
	vm_push(&S->vm, YASL_USERDATA(ud_new(data, tag, mt, destructor)));
	return YASL_SUCCESS;
}

int YASL_pushuserpointer(struct YASL_State *S, void *userpointer) {
	vm_push((struct VM *) S, YASL_USERPTR(userpointer));
	return YASL_SUCCESS;
}

int YASL_pushszstring(struct YASL_State *S, const char *value) {
	vm_pushstr((struct VM *) S, YASL_String_new_sized_heap(0, strlen(value), value));
	return YASL_SUCCESS;
}

int YASL_pushlitszstring(struct YASL_State *S, const char *value) {
	vm_pushstr((struct VM *) S, YASL_String_new_sized(strlen(value), value));
	return YASL_SUCCESS;
}

int YASL_pushstring(struct YASL_State *S, const char *value, const size_t size) {
	vm_pushstr((struct VM *) S, YASL_String_new_sized_heap(0, size, value));
	return YASL_SUCCESS;
}

int YASL_pushlitstring(struct YASL_State *S, const char *value, const size_t size) {
	vm_pushstr((struct VM *) S, YASL_String_new_sized(size, value));
	return YASL_SUCCESS;
}

int YASL_pushcfunction(struct YASL_State *S, int (*value)(struct YASL_State *), int num_args) {
	vm_push((struct VM *) S, YASL_CFN(value, num_args));
	return YASL_SUCCESS;
}

int YASL_pushtable(struct YASL_State *S) {
	struct RC_UserData *table = rcht_new();
	vm_push(&S->vm, YASL_TABLE(table));
	return YASL_SUCCESS;
}

int YASL_pushobject(struct YASL_State *S, struct YASL_Object *obj) {
	if (!obj) return YASL_ERROR;
	vm_push((struct VM *) S, *obj);
	// free(obj); // TODO: delete properly
	return YASL_SUCCESS;
}

struct YASL_Object *YASL_popobject(struct YASL_State *S) {
	return &S->vm.stack[S->vm.sp--];
}

int YASL_pop(struct YASL_State *S) {
	// TODO: check the stack pointer?
	vm_pop(&S->vm);
	return YASL_SUCCESS;
}

int YASL_settable(struct YASL_State *S) {
	struct YASL_Object value = vm_pop(&S->vm);
	struct YASL_Object key = vm_pop(&S->vm);
	struct YASL_Object table = vm_pop(&S->vm);

	if (!YASL_ISTABLE(table))
		return YASL_ERROR;
	YASL_Table_insert(YASL_GETTABLE(table), key, value);

	return YASL_SUCCESS;
}

int YASL_UserData_gettag(struct YASL_Object *obj) {
	return obj->value.uval->tag;
}

void *YASL_UserData_getdata(struct YASL_Object *obj) {
	return obj->value.uval->data;
}

bool YASL_top_isundef(struct YASL_State *S) {
	return vm_isundef(&S->vm);
}

bool YASL_top_isboolean(struct YASL_State *S) {
	return vm_isbool(&S->vm);
}

bool YASL_top_isdouble(struct YASL_State *S) {
	return vm_isfloat(&S->vm);
}

bool YASL_top_isfloat(struct YASL_State *S) {
	return vm_isfloat(&S->vm);
}

bool YASL_top_isinteger(struct YASL_State *S) {
	return vm_isint(&S->vm);
}

bool YASL_top_isstring(struct YASL_State *S) {
	return vm_isstr(&S->vm);
}

bool YASL_top_islist(struct YASL_State *S) {
	return vm_islist(&S->vm);
}

bool YASL_top_istable(struct YASL_State *S) {
	return vm_istable(&S->vm);
}

bool YASL_top_isuserdata(struct YASL_State *S, int tag) {
	return vm_isuserdata(&S->vm) && YASL_GETUSERDATA(vm_peek(&S->vm))->tag == tag;
}

bool YASL_top_isuserpointer(struct YASL_State *S) {
	return vm_isuserptr(&S->vm);
}

bool YASL_top_peekboolean(struct YASL_State *S) {
	if (YASL_top_isboolean(S)) {
		return vm_popbool(&S->vm);
	}
	return false;
}

bool YASL_top_popboolean(struct YASL_State *S) {
	if (YASL_top_isboolean(S)) {
		return (bool)YASL_GETBOOL(vm_pop(&S->vm));
	}
	return false;
}

yasl_float YASL_top_peekdouble(struct YASL_State *S) {
	if (YASL_top_isfloat(S)) {
		return vm_peekfloat(&S->vm);
	}
	return 0.0;
}

yasl_float YASL_top_peekfloat(struct YASL_State *S) {
	if (YASL_top_isfloat(S)) {
		return vm_peekfloat(&S->vm);
	}
	return 0.0;
}

yasl_float YASL_top_popdouble(struct YASL_State *S) {
	if (YASL_top_isfloat(S)) {
		return vm_popfloat(&S->vm);
	}
	return 0.0;
}

yasl_float YASL_top_popfloat(struct YASL_State *S) {
	if (YASL_top_isfloat(S)) {
		return vm_popfloat(&S->vm);
	}
	return 0.0;
}

yasl_int YASL_top_peekinteger(struct YASL_State *S) {
	if (YASL_top_isinteger(S)) {
		return vm_peekint(&S->vm);
	}
	return 0;
}

yasl_int YASL_top_popinteger(struct YASL_State *S) {
	if (YASL_top_isinteger(S)) {
		return vm_popint(&S->vm);
	}
	return 0;
}

char *YASL_top_peekcstring(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) return NULL;

	struct YASL_Object obj = vm_peek(&S->vm);
	char *tmp = (char *) malloc(YASL_String_len(obj.value.sval) + 1);

	memcpy(tmp, obj.value.sval->str + obj.value.sval->start, YASL_String_len(obj.value.sval));
	tmp[YASL_String_len(obj.value.sval)] = '\0';

	return tmp;
}

char *YASL_top_popcstring(struct YASL_State *S) {
	char *tmp = YASL_top_peekcstring(S);
	YASL_pop(S);
	return tmp;
}

void *YASL_top_peekuserdata(struct YASL_State *S) {
	return YASL_GETUSERDATA(vm_peek(&S->vm))->data;
}

void *YASL_top_popuserdata(struct YASL_State *S) {
	return YASL_GETUSERDATA(vm_pop(&S->vm))->data;
}

void *YASL_top_peekuserpointer(struct YASL_State *S) {
	return YASL_GETUSERPTR(vm_peek(&S->vm));
}

void *YASL_top_popuserpointer(struct YASL_State *S) {
	return YASL_GETUSERPTR(vm_pop(&S->vm));
}
