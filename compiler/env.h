#ifndef YASL_ENV_H_
#define YASL_ENV_H_

#include "data-structures/YASL_Table.h"

struct Scope {
    struct Scope *parent;
    struct YASL_Table vars;
    // TODO: keep track of which variables need to be closed over...?
};

struct Env {
	struct Env *parent;
	struct Scope *scope;
	struct YASL_Table upval_indices;  // Which order to store the upvalues in the closure
	struct YASL_Table upval_values;   // What index to look at in the above scope
	bool isclosure;
	bool usedinclosure;
};

struct Scope *scope_new(struct Scope *const scope);
void scope_del(struct Scope *const scope);
void scope_del_current_only(struct Scope *const scope);

size_t scope_len(const struct Scope *const scope);
bool scope_contains_cur_only(const struct Scope *const scope, const char *const name);
bool scope_contains(const struct Scope *const scope, const char *const name);
int64_t scope_get(const struct Scope *const scope, const char *const name);
int64_t scope_decl_var(struct Scope *const scope, const char *const name);
bool scope_used_in_closure(const struct Scope *const scope);
void scope_make_const(struct Scope *const scope, const char *const name);

bool env_contains(const struct Env *const env, const char *const name);
bool env_contains_cur_only(const struct Env *const env, const char *const name);
struct Env *env_new(struct Env *const env);
int64_t env_resolve_upval_index(struct Env *const env, const char *const name);
int64_t env_resolve_upval_value(struct Env *const env, const char *const name);
void env_del(struct Env *const env);

#endif
