#include <compiler/compiler.h>
#include <interpreter/VM.h>
#include <interpreter/table_methods.h>
#include <interpreter/YASL_Object.h>
#include <interpreter/YASL_string.h>
#include <interpreter/userdata.h>
#include <bytebuffer/bytebuffer.h>
#include "yasl.h"
#include "yasl_state.h"
#include "compiler/compiler.h"
#include "interpreter/VM.h"
#include "compiler/lexinput.h"
//#include "interpreter/YASL_object/YASL_Object.h"

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

struct YASL_State *YASL_newstate(char *filename) {
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


struct YASL_State *YASL_newstate_bb(char *buf, int len) {
	struct YASL_State *S = (struct YASL_State *) malloc(sizeof(struct YASL_State));

	struct LEXINPUT *lp = lexinput_new_bb(buf, len);
	struct Compiler tcomp = NEW_COMPILER(lp);
	S->compiler = tcomp;
	S->compiler.header->count = 16;
	S->compiler.num = 0;

	vm_init((struct VM *) S, NULL, -1, 1);
	return S;
}

void YASL_resetstate_bb(struct YASL_State *S, char *buf, size_t len) {
	S->compiler.status = YASL_SUCCESS;
	S->compiler.parser.status = YASL_SUCCESS;
	lex_cleanup(&S->compiler.parser.lex);
	S->compiler.parser.lex = NEW_LEXER(lexinput_new_bb(buf, len));
	S->compiler.code->count = 0;
	S->compiler.buffer->count = 0;
	// S->compiler.header->count = 16;
	// table_del_string_int(S->compiler.strings);
	// S->compiler.strings = table_new();
	if (S->vm.code)	free(S->vm.code);
	S->vm.code = NULL;
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
	struct YASL_Object value = table_search_string_int(S->compiler.strings, name, strlen(name));
	if (value.type == Y_END) {
		YASL_COMPILE_DEBUG_LOG("%s\n", "caching string");
		table_insert_string_int(S->compiler.strings, name, strlen(name), S->compiler.header->count);
		bb_intbytes8(S->compiler.header, strlen(name));
		bb_append(S->compiler.header, (unsigned char *) name, strlen(name));
	}
	int64_t index = env_decl_var(S->compiler.globals, name, strlen(name));
	return YASL_SUCCESS;
}

static inline int is_const(int64_t value) {
	const uint64_t MASK = 0x8000000000000000;
	return (MASK & value) != 0;
}

int YASL_setglobal(struct YASL_State *S, const char *name) {

	if (!env_contains(S->compiler.globals, name, strlen(name))) return YASL_ERROR;

	int64_t index = env_get(S->compiler.globals, name, strlen(name));
	if (is_const(index)) return YASL_ERROR;

	// int64_t num_globals = S->compiler.globals->vars->count;

	// S->vm.globals = realloc(S->vm.globals, num_globals * sizeof(YASL_Object));

	String_t *string = str_new_sized(strlen(name), name);
	struct YASL_Object obj = table_search(S->vm.globals[0], YASL_STR(string));
	dec_ref(&obj);
	// inc_ref(S->vm.stack + S->vm.sp);
	table_insert(S->vm.globals[0], YASL_STR(string), vm_peek((struct VM *) S));
	S->vm.sp--;
//	vm_pop((struct VM *) S);
	// inc_ref(S->vm.globals + index);

	return YASL_SUCCESS;
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
	VM_PUSH((struct VM *) S, YASL_STR(str_new_sized(strlen(value), value)));
	return YASL_SUCCESS;
}

int YASL_pushcstring(struct YASL_State *S, char *value) {
	VM_PUSH((struct VM *) S, YASL_STR(str_new_sized(strlen(value), value)));
	return YASL_SUCCESS;
}

int YASL_pushuserpointer(struct YASL_State *S, void *userpointer) {
	VM_PUSH((struct VM *) S, YASL_USERPTR(userpointer));
	return YASL_SUCCESS;
}

int YASL_pushstring(struct YASL_State *S, char *value, int64_t size) {
	VM_PUSH((struct VM *) S, YASL_STR(str_new_sized_heap(0, size, value)));
	return YASL_SUCCESS;
}

int YASL_pushcfunction(struct YASL_State *S, int (*value)(struct YASL_State *), int num_args) {
	VM_PUSH((struct VM *) S, YASL_CFN(value, num_args));
	return YASL_SUCCESS;
}

int YASL_pushobject(struct YASL_State *S, struct YASL_Object *obj) {
	if (!obj) return YASL_ERROR;
	vm_push((struct VM *) S, *obj);
	free(obj); // TODO: delete properly
	return YASL_SUCCESS;
}

struct YASL_Object *YASL_popobject(struct YASL_State *S) {
	return &S->vm.stack[S->vm.sp--];
}

int YASL_Table_set(struct YASL_Object *table, struct YASL_Object *key, struct YASL_Object *value) {
	if (!table || !key || !value) return YASL_ERROR;

	// TODO: fix this to YASL_isTable(table)
	if (table->type != Y_TABLE)
		return YASL_ERROR;
	table_insert(YASL_GETTABLE(*table), *key, *value);

	return YASL_SUCCESS;
}

int YASL_UserData_gettag(struct YASL_Object *obj) {
	return obj->value.uval->tag;
}

void *YASL_UserData_getdata(struct YASL_Object *obj) {
	return obj->value.uval->data;
}

struct YASL_Object *YASL_LiteralString(const char *str) {
	return YASL_String(str_new_sized(strlen(str), str));
}

struct YASL_Object *YASL_CString(char *str) {
	return YASL_String(str_new_sized(strlen(str), str));
}



int YASL_isundef(struct YASL_Object *obj) {
	return obj->type != Y_UNDEF;
}


int YASL_isboolean(struct YASL_Object *obj) {
	return obj->type != Y_BOOL;
}


int YASL_isdouble(struct YASL_Object *obj) {
	return obj->type != Y_FLOAT;
}


int YASL_isinteger(struct YASL_Object *obj) {
	return obj->type != Y_INT;
}

int YASL_isstring(struct YASL_Object *obj) {
	return obj->type != Y_STR && obj->type != Y_STR_W;
}

int YASL_islist(struct YASL_Object *obj) {
	return obj->type != Y_LIST && obj->type != Y_LIST_W;
}

int YASL_istable(struct YASL_Object *obj) {
	return obj->type != Y_TABLE && obj->type != Y_TABLE_W;
}

int YASL_isfunction(struct YASL_Object *obj);


int YASL_iscfunction(struct YASL_Object *obj);


int YASL_isuserdata(struct YASL_Object *obj, int tag) {
	if (YASL_ISUSERDATA(*obj) && obj->value.uval->tag == tag) {
		return YASL_SUCCESS;
	}
	return YASL_ERROR;
}


int YASL_isuserpointer(struct YASL_Object *obj);


int YASL_getboolean(struct YASL_Object *obj);


double YASL_getdouble(struct YASL_Object *obj);


int64_t YASL_getinteger(struct YASL_Object *obj);


char *YASL_getcstring(struct YASL_Object *obj) {
	if (YASL_isstring(obj) != YASL_SUCCESS) return NULL;

	char *tmp = (char *) malloc(yasl_string_len(obj->value.sval) + 1);

	memcpy(tmp, obj->value.sval->str + obj->value.sval->start, yasl_string_len(obj->value.sval));
	tmp[yasl_string_len(obj->value.sval)] = '\0';

	return tmp;
}

size_t YASL_getstringlen(struct YASL_Object *obj) {
	if (YASL_isstring(obj) != YASL_SUCCESS) return 0;

	return yasl_string_len(obj->value.sval);
}

char *YASL_getstring(struct YASL_Object *obj);


// int (*)(struct YASL_State) *YASL_getcfunction(struct YASL_Object *obj);


void *YASL_getuserdata(struct YASL_Object *obj) {
	if (obj->type == Y_USERDATA || obj->type == Y_USERDATA_W) {
		return obj->value.uval->data;
	}
	return NULL;
}


void *YASL_getuserpointer(struct YASL_Object *obj);

