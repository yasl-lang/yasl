#include <compiler/compiler/compiler.h>
#include <interpreter/VM/VM.h>
#include "yasl.h"
#include "compiler.h"
#include "VM.h"
#include "YASL_Object.h"

struct YASL_State {
    Compiler *compiler;
    VM *vm;
};

struct YASL_State *YASL_newstate(char *filename) {
    struct YASL_State *S = malloc(sizeof(struct YASL_State));

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return NULL;  // Can't open file.
    }

    fseek(fp, 0, SEEK_SET);
    S->compiler = compiler_new(parser_new(lex_new(fp)));

    S->vm = vm_new(NULL, -1, 256); // TODO: decide proper number of globals
    return S;
}

int YASL_delstate(struct YASL_State *S) {
    if (S->compiler) compiler_del(S->compiler);
    if (S->vm) vm_del(S->vm);
    free(S);
    return YASL_SUCCESS;
}

int YASL_execute(struct YASL_State *S) {
    char *bc = compile(S->compiler);
    if (!bc) return S->compiler->status;

    int64_t entry_point = *((int64_t*)bc);
    int64_t num_globals = *((int64_t*)bc+1);

    S->vm->pc0 = S->vm->pc = entry_point;
    S->vm->code = bc;

    vm_run(S->vm);  // TODO: error handling for runtime errors.
    return YASL_SUCCESS;
}


int YASL_declglobal(struct YASL_State *S, char *name) {
    env_decl_var(S->compiler->globals, name, strlen(name));
    return YASL_SUCCESS;
}

static inline int is_const(int64_t value) {
    const uint64_t MASK = 0x8000000000000000;
    return (MASK & value) != 0;
}

int YASL_setglobal(struct YASL_State *S, char *name) {
    if (!env_contains(S->compiler->globals, name, strlen(name))) return YASL_ERROR;

    int64_t index = env_get(S->compiler->globals, name, strlen(name));
    if (is_const(index)) return YASL_ERROR;

    // int64_t num_globals = S->compiler->globals->vars->count;

    // S->vm->globals = realloc(S->vm->globals, num_globals * sizeof(YASL_Object));

    inc_ref(&S->vm->stack[S->vm->sp]);
    S->vm->globals[index] = vm_pop(S->vm);
}


int YASL_pushundef(struct YASL_State *S) {
    vm_push(S->vm, YASL_Undef());
}

int YASL_pushfloat(struct YASL_State *S, double value) {
    vm_push(S->vm, YASL_Float(value));
}

int YASL_pushinteger(struct YASL_State *S, int64_t value) {
    vm_push(S->vm, YASL_Integer(value));
}

int YASL_pushboolean(struct YASL_State *S, int value) {
    vm_push(S->vm, YASL_Boolean(value));
}


int YASL_pushcstring(struct YASL_State *S, char *value) {
    unsigned char *tmp = malloc(strlen(value) + 1);
    memcpy(tmp, value, strlen(value) + 1);
    vm_push(S->vm, YASL_String(str_new_sized(strlen(tmp), tmp)));
}

/*
int YASL_pushstring(struct YASL_State *S, char *value, int64_t size);


int YASL_pushcfunction(struct YASL_State *S, int (*value)(struct YASL_State *));


int YASL_isundef(YASL_Object *obj);


int YASL_isboolean(YASL_Object *obj);


int YASL_isdouble(YASL_Object *obj);


int YASL_isinteger(YASL_Object *obj);


int YASL_isstring(YASL_Object *obj);


int YASL_islist(YASL_Object *obj);


int YASL_istable(YASL_Object *obj);


int YASL_isfunction(YASL_Object *obj);


int YASL_iscfunction(YASL_Object *obj);


int YASL_isuserdata(YASL_Object *obj);


int YASL_isuserpointer(YASL_Object *obj);


int YASL_getboolean(YASL_Object *obj);


double YASL_getdouble(YASL_Object *obj);


int64_t YASL_getinteger(YASL_Object *obj);


char *YASL_getcstring(YASL_Object *obj);


char *YASL_getstring(YASL_Object *obj);


int (*)(struct YASL_State) *YASL_getcfunction(YASL_Object *obj);


void *YASL_getuserdata(YASL_Object *obj);


void *YASL_getuserpointer(YASL_Object *obj);

 */