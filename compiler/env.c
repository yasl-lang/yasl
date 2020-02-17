#include "env.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "data-structures/YASL_String.h"

struct Env *env_new(struct Env *const parent) {
	struct Env *env = (struct Env *)malloc(sizeof(struct Env));
	env->scope = NULL;
	env->upvals = NEW_TABLE();
	env->usedinclosure = false;
	env->isclosure = false;
	env->parent = parent;
	return env;
}

void env_del(struct Env *const env) {
	if (env == NULL) return;
	scope_del(env->scope);
	for (size_t i = 0; i < env->upvals.size; i++) {
		struct YASL_Table_Item *item = &env->upvals.items[i];
		if (item->key.type != Y_END && item->key.type != Y_UNDEF) {
			str_del(item->key.value.sval);
		}
	}
	free(env->upvals.items);
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

bool scope_contains_cur_scope(const struct Scope *const scope, const char *const name) {
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
