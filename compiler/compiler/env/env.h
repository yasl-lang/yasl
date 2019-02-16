//
// Created by thiabaud on 01/05/18.
//
#include "hashtable/hashtable.h"
#include "interpreter/YASL_string/YASL_string.h"
#include <string.h>

struct Env_s {
    struct Env_s *parent;
    struct Table *vars;
};

typedef struct Env_s Env_t;

Env_t *env_new(Env_t *env);
void env_del(Env_t *env);
void env_del_current_only(Env_t *env);

size_t env_len(Env_t *env);
int env_contains_cur_scope(Env_t *env, char *name, size_t name_len);
int env_contains(Env_t *env, char *name, size_t name_len);
int64_t env_get(Env_t *env, char *name, size_t name_len);
void env_decl_var(Env_t *env, char *name, size_t name_len);
void env_make_const(Env_t *env, char *name, size_t name_len);
