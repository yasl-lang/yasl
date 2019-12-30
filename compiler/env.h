#pragma once

#include "data-structures/YASL_Table.h"

struct Env {
    struct Env *parent;
    struct YASL_Table *vars;
};

struct Env *env_new(struct Env *env);
void env_del(struct Env *env);
void env_del_current_only(struct Env *env);

size_t env_len(const struct Env *const env);
int env_contains_cur_scope(const struct Env *const env, const char *const name, const size_t name_len);
int env_contains(const struct Env *const env, const char *const name, const size_t name_len);
int64_t env_get(const struct Env *const env, const char *const name, const size_t name_len);
int64_t env_decl_var(struct Env *const env, const char *const name, const size_t name_len);
void env_make_const(struct Env *const env,  const char *const name, const size_t name_len);
