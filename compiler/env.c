#include "env.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <interpreter/YASL_Object.h>

#include "debug.h"
#include "data-structures/YASL_String.h"

struct Env *env_new(struct Env *const parent) {
	struct Env *env = (struct Env *)malloc(sizeof(struct Env));
	env->scope = NULL;
	env->upval_indices = NEW_TABLE();
	env->upval_values = NEW_TABLE();
	env->usedinclosure = false;
	env->isclosure = false;
	env->parent = parent;
	return env;
}

static void YASL_Table_string_int_cleanup(struct YASL_Table *const table) {
	for (size_t i = 0; i < table->size; i++) {
		struct YASL_Table_Item *item = &table->items[i];
		if (item->key.type != Y_END && item->key.type != Y_UNDEF) {
			str_del(item->key.value.sval);
		}
	}
	free(table->items);
}

void env_del(struct Env *const env) {
	if (env == NULL) return;
	scope_del(env->scope);
	YASL_Table_string_int_cleanup(&env->upval_indices);
	YASL_Table_string_int_cleanup(&env->upval_values);
	env_del(env->parent);
	free(env);
}

struct Scope *scope_new(struct Scope *const parent) {
	struct Scope *scope = (struct Scope *)malloc(sizeof(struct Scope));
	scope->parent = parent;
	scope->vars = NEW_TABLE();
	return scope;
}

void scope_del(struct Scope *const scope) {
	if (scope == NULL) return;
	scope_del(scope->parent);
	free(scope->parent);
	scope_del_cur_only(scope);
}

void scope_del_cur_only(struct Scope *const scope) {
	YASL_Table_string_int_cleanup(&scope->vars);
	free(scope);
}

size_t scope_num_vars_cur_only(const struct Scope *const scope) {
	return scope->vars.count;
}

size_t scope_len(const struct Scope *const scope) {
	if (scope == NULL)
		return 0;
	return scope_num_vars_cur_only(scope) + scope_len(scope->parent);
}

bool scope_contains_cur_only(const struct Scope *const scope, const char *const name) {
	return YASL_Table_contains_zstring_int(&scope->vars, name);
}

bool scope_contains(const struct Scope *const scope, const char *const name) {
	if (scope == NULL) return false;

	const bool contains = scope_contains_cur_only(scope, name);
	if (!contains && scope->parent == NULL) {
		return false;
	}
	if (!contains)
		return scope_contains(scope->parent, name);
	return true;
}

bool env_contains(const struct Env *env, const char *const name) {
	while (env != NULL) {
		if (scope_contains(env->scope, name)) return true;
		env = env->parent;
	}
	return false;
}

bool env_contains_cur_only(const struct Env *const env, const char *const name) {
	return scope_contains(env->scope, name);
}

static int is_const(const int64_t value) {
	const uint64_t MASK = 0x8000000000000000;
	return (MASK & value) != 0;
}

static inline int64_t get_index(const int64_t value) {
	return is_const(value) ? ~value : value;
}

static int64_t env_add_upval_kv(struct Env *env, const char *const name, yasl_int value) {
	YASL_Table_insert_zstring_int(&env->upval_values, name, value);
	int64_t index = env->upval_indices.count;
	YASL_Table_insert_zstring_int(&env->upval_indices, name, index);
	return index;
}

static int64_t env_add_upval(struct Env *env, struct Scope *stack, const char *const name) {
	env->isclosure = true;

	if (!env->parent && scope_contains(stack, name)) {
		yasl_int value = get_index(scope_get(stack, name));
		return env_add_upval_kv(env, name, value);
	}

	YASL_ASSERT(env->parent, "Parent cannot be null.");

	if (!env_contains_cur_only(env->parent, name) && !YASL_Table_contains_zstring_int(&env->parent->upval_indices, name)) {
		env_add_upval(env->parent, stack, name);
	}

	if (env_contains_cur_only(env->parent, name)) {
		env->parent->usedinclosure = true;
		yasl_int value = get_index(scope_get(env->parent->scope, name));
		return env_add_upval_kv(env, name, value);
	}

	const struct YASL_Object res = YASL_Table_search_zstring_int(&env->parent->upval_indices, name);
	if (obj_isint(&res)) {
		yasl_int value = obj_getint(&res);
		return env_add_upval_kv(env, name, value < 0 ? value : ~value);;
	}

	YASL_UNREACHED();
	return 0;
}

int64_t env_resolve_upval_index(struct Env *const env, struct Scope *stack, const char *const name) {
	struct YASL_Object value = YASL_Table_search_zstring_int(&env->upval_indices, name);

	if (value.type == Y_INT) {
		return value.value.ival;
	}

	return env_add_upval(env, stack, name);
}

// Assumes that the upval in question is already in the upvals for env.
int64_t env_resolve_upval_value(struct Env *const env, const char *const name) {
	struct YASL_Object value = YASL_Table_search_zstring_int(&env->upval_values, name);

	YASL_ASSERT(value.type == Y_INT, "Value must be found in upvals for env.");

	return obj_getint(&value);
}

int64_t scope_get(const struct Scope *const scope, const char *const name) {
	struct YASL_Object value = YASL_Table_search_zstring_int(&scope->vars, name);
	if (value.type == Y_END && scope->parent == NULL) {
		YASL_ASSERT(false, "Lookup should not fail.");
	}
	if (value.type == Y_END) return scope_get(scope->parent, name);
	return value.value.ival;
}

int64_t scope_decl_var(struct Scope *const scope, const char *const name) {
	YASL_Table_insert_zstring_int(&scope->vars, name, scope_len(scope));
	return scope_len(scope);
}

static struct YASL_Table *get_closest_scope_with_var(struct Scope *const scope, const char *const name) {
	return YASL_Table_contains_zstring_int(&scope->vars, name) ? &scope->vars : get_closest_scope_with_var(scope->parent, name);
}

void scope_make_const(struct Scope *const scope, const char *const name) {
	struct YASL_Table *ht = get_closest_scope_with_var(scope, name);
	YASL_Table_insert_zstring_int(ht, name, ~YASL_Table_search_zstring_int(ht, name).value.ival);
}
