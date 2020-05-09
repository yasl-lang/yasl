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

void env_del(struct Env *const env) {
	if (env == NULL) return;
	scope_del(env->scope);
	for (size_t i = 0; i < env->upval_indices.size; i++) {
		struct YASL_Table_Item *item = &env->upval_indices.items[i];
		if (item->key.type != Y_END && item->key.type != Y_UNDEF) {
			dec_ref(&item->key);
		}
	}
	free(env->upval_indices.items);
	for (size_t i = 0; i < env->upval_values.size; i++) {
		struct YASL_Table_Item *item = &env->upval_values.items[i];
		if (item->key.type != Y_END && item->key.type != Y_UNDEF) {
			dec_ref(&item->key);
		}
	}
	free(env->upval_values.items);
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
	scope_del_current_only(scope);
}

void scope_del_current_only(struct Scope *const scope) {
	for (size_t i = 0; i < scope->vars.size; i++) {
		struct YASL_Table_Item *item = &scope->vars.items[i];
		if (item->key.type != Y_END && item->key.type != Y_UNDEF) {
			str_del(item->key.value.sval);
		}
	}
	free(scope->vars.items);
	free(scope);
}

size_t scope_len(const struct Scope *const scope) {
	if (scope == NULL) return 0;
	return scope->vars.count + scope_len(scope->parent);
}

bool scope_contains_cur_only(const struct Scope *const scope, const char *const name) {
	const size_t name_len = strlen(name);
	struct YASL_String *string = YASL_String_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string); // (struct YASL_Object) { .value.sval = string, .type = Y_STR };

	struct YASL_Object value = YASL_Table_search(&scope->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END) {
		return false;
	}
	return true;
}

bool scope_contains(const struct Scope *const scope, const char *const name) {
	const size_t name_len = strlen(name);
	if (scope == NULL) return false;
	struct YASL_String *string = YASL_String_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string);

	struct YASL_Object value = YASL_Table_search(&scope->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END && scope->parent == NULL) {
		return false;
	}
	if (value.type == Y_END) return scope_contains(scope->parent, name);
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

static int64_t env_add_upval(struct Env *env, const char *const name) {
	struct YASL_String *string = YASL_String_new_sized_heap(0, strlen(name), copy_char_buffer(strlen(name), name));
	struct YASL_Object key = YASL_STR(string);
	struct YASL_Object res;
	YASL_ASSERT(env->parent, "Parent cannot be null in add_upval.");

	env->isclosure = true;
	if (!env_contains_cur_only(env->parent, name) && (YASL_Table_search(&env->parent->upval_indices, key)).type != Y_INT) {
		env_add_upval(env->parent, name);
	}

	if (env_contains_cur_only(env->parent, name)) {
		env->parent->usedinclosure = true;
		yasl_int value = get_index(scope_get(env->parent->scope, name));
		YASL_Table_insert(&env->upval_values, key, YASL_INT(value));
		int64_t index = env->upval_indices.count;
		YASL_Table_insert(&env->upval_indices, key, YASL_INT(index));
		return index;
	}

	if ((res = YASL_Table_search(&env->parent->upval_indices, key)).type == Y_INT) {
		yasl_int value = res.value.ival;
		YASL_Table_insert(&env->upval_values, key, YASL_INT(value < 0 ? value : ~value));
		int64_t index = env->upval_indices.count;
		YASL_Table_insert(&env->upval_indices, key, YASL_INT(index));
		return index;
	}

	YASL_ASSERT(false, "shouldn't reach here.");
	return 0;
}

int64_t env_resolve_upval_index(struct Env *const env, const char *const name) {
	struct YASL_String *string = YASL_String_new_sized_heap(0, strlen(name), copy_char_buffer(strlen(name), name));
	struct YASL_Object key = YASL_STR(string);

	struct YASL_Object value = YASL_Table_search(&env->upval_indices, key);
	str_del(key.value.sval);

	if (value.type == Y_INT) {
		return value.value.ival;
	}

	return env_add_upval(env, name);
}

// Assumes that the upval in question is already in the upvals for env.
int64_t env_resolve_upval_value(struct Env *const env, const char *const name) {
	struct YASL_String *string = YASL_String_new_sized_heap(0, strlen(name), copy_char_buffer(strlen(name), name));
	struct YASL_Object key = YASL_STR(string);

	struct YASL_Object value = YASL_Table_search(&env->upval_values, key);

	YASL_ASSERT(value.type == Y_INT, "Value must be found in upvals for env.");

	str_del(key.value.sval);
	return YASL_GETINT(value);
}

int64_t scope_get(const struct Scope *const scope, const char *const name) {
	const size_t name_len = strlen(name);
	struct YASL_String *string = YASL_String_new_sized_heap(0, name_len, copy_char_buffer(name_len + 1, name));
	struct YASL_Object key = YASL_STR(string);

	struct YASL_Object value = YASL_Table_search(&scope->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END && scope->parent == NULL) {
		printf("error in scope_get with key: ");
		print(key);
		exit(EXIT_FAILURE);
	}
	if (value.type == Y_END) return scope_get(scope->parent, name);
	return value.value.ival;
}

int64_t scope_decl_var(struct Scope *const scope, const char *const name) {
	const size_t name_len = strlen(name);
	struct YASL_String *string = YASL_String_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string);
	struct YASL_Object value = YASL_INT((long) scope_len(scope));
	YASL_Table_insert(&scope->vars, key, value);
	return scope_len(scope);
}

static struct YASL_Table *get_closest_scope_with_var(struct Scope *const scope, const char *const name, const size_t name_len) {
	struct YASL_Object key = YASL_Table_search_string_int(&scope->vars, name, name_len);
	return key.type != Y_END ? &scope->vars : get_closest_scope_with_var(scope->parent, name, name_len);
}

void scope_make_const(struct Scope *const scope, const char *const name) {
	const size_t name_len = strlen(name);
	struct YASL_Table *ht = get_closest_scope_with_var(scope, name, name_len);
	YASL_Table_insert_string_int(ht, name, name_len, ~YASL_Table_search_string_int(ht, name, name_len).value.ival);
}
