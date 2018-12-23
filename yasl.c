#include <compiler/compiler/compiler.h>
#include <interpreter/VM/VM.h>
#include <interpreter/table/table_methods.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include <interpreter/YASL_string/YASL_string.h>
#include <interpreter/userdata/userdata.h>
#include "yasl.h"
#include "yasl_state.h"
#include "compiler.h"
#include "VM.h"
#include "YASL_Object.h"

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
    unsigned char *bc = compile(S->compiler);
    if (!bc) return S->compiler->status;

    int64_t entry_point = *((int64_t*)bc);
    // TODO: use this in VM.
    // int64_t num_globals = *((int64_t*)bc+1);

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

    dec_ref(S->vm->globals + index);
    S->vm->globals[index] = vm_pop(S->vm);
    inc_ref(S->vm->globals + index);

    return YASL_SUCCESS;
}


int YASL_pushundef(struct YASL_State *S) {
    vm_push(S->vm, YASL_UNDEF());
    return YASL_SUCCESS;
}

int YASL_pushfloat(struct YASL_State *S, double value) {
    vm_push(S->vm, YASL_FLOAT(value));
    return YASL_SUCCESS;
}

int YASL_pushinteger(struct YASL_State *S, int64_t value) {
    vm_push(S->vm, YASL_INT(value));
    return YASL_SUCCESS;
}

int YASL_pushboolean(struct YASL_State *S, int value) {
    vm_push(S->vm, YASL_BOOL(value));
    return YASL_SUCCESS;
}

int YASL_pushliteralstring(struct YASL_State *S, char *value) {
    vm_push(S->vm, YASL_STR(str_new_sized_heap(0, strlen(value), value)));
    return YASL_SUCCESS;
}

int YASL_pushcstring(struct YASL_State *S, char *value) {
    vm_push(S->vm, YASL_STR(str_new_sized(strlen(value), value)));
    return YASL_SUCCESS;
}

int YASL_pushuserpointer(struct YASL_State *S, void *userpointer) {
    vm_push(S->vm, YASL_USERPTR(userpointer));
    return YASL_SUCCESS;
}

int YASL_pushstring(struct YASL_State *S, char *value, int64_t size) {
    vm_push(S->vm, YASL_STR(str_new_sized(size, value)));
    return YASL_SUCCESS;
}

int YASL_pushcfunction(struct YASL_State *S, int (*value)(struct YASL_State *), int num_args) {
    vm_push(S->vm, YASL_CFN(value, num_args));
    return YASL_SUCCESS;
}

int YASL_pushobject(struct YASL_State *S, struct YASL_Object *obj) {
    if (!obj) return YASL_ERROR;
    vm_push(S->vm, *obj);
    free(obj);
    return YASL_SUCCESS;
}

struct YASL_Object *YASL_popobject(struct YASL_State *S) {
    return &S->vm->stack[S->vm->sp--];
}

int YASL_Table_set(struct YASL_Object *table, struct YASL_Object *key, struct YASL_Object *value) {
    if (!table || !key || !value) return YASL_ERROR;

    // TODO: fix this to YASL_isTable(table)
    if (table->type != Y_TABLE)
        return YASL_ERROR;
    table_insert(YASL_GETTBL(*table), *key, *value);

    return YASL_SUCCESS;
}

int YASL_UserData_gettag(struct YASL_Object *obj) {
    return obj->value.uval->tag;
}

void *YASL_UserData_getdata(struct YASL_Object *obj) {
    return obj->value.uval->data;
}

struct YASL_Object *YASL_LiteralString(char *str) {
    return YASL_String(str_new_sized_heap(0, strlen(str), str));
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
    return obj->type != Y_FLOAT64;
}


int YASL_isinteger(struct YASL_Object *obj) {
    return obj->type != Y_INT64;
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

    char *tmp = malloc(yasl_string_len(obj->value.sval) + 1);

    memcpy(tmp, obj->value.sval->str + obj->value.sval->start, yasl_string_len(obj->value.sval));
    tmp[yasl_string_len(obj->value.sval)] = '\0';

    return tmp;
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

