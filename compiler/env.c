#include "env.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "data-structures/YASL_String.h"

struct Env *env_new(struct Env *const parent) {
	struct Env *env = (struct Env *)malloc(sizeof(struct Env));
	env->parent = parent;
	env->vars = NEW_TABLE();
	env->used_in_closure = false;
	return env;
}

void env_del(struct Env *const env) {
	if (env == NULL) return;
	env_del(env->parent);
	free(env->parent);
	env_del_current_only(env);
}

void env_del_current_only(struct Env *const env) {
	for (size_t i = 0; i < env->vars.size; i++) {
		struct YASL_Table_Item *item = &env->vars.items[i];
		if (item->key.type != Y_END && item->key.type != Y_UNDEF) {
			str_del(item->key.value.sval);
		}
	}
	free(env->vars.items);
	free(env);
}

size_t env_len(const struct Env *const env) {
	if (env == NULL) return 0;
	return env->vars.count + env_len(env->parent);
}

int env_contains_cur_scope(const struct Env *const env, const char *const name) {
	const size_t name_len = strlen(name);
	struct YASL_String *string = YASL_String_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string); // (struct YASL_Object) { .value.sval = string, .type = Y_STR };

	struct YASL_Object value = YASL_Table_search(&env->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END) {
		return 0;
	}
	return 1;
}

int env_contains(const struct Env *const env, const char *const name) {
	const size_t name_len = strlen(name);
	if (env == NULL) return 0;
	struct YASL_String *string = YASL_String_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string);

	struct YASL_Object value = YASL_Table_search(&env->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END && env->parent == NULL) {
		return 0;
	}
	if (value.type == Y_END) return env_contains(env->parent, name);
	return 1;
}

int64_t env_get(const struct Env *const env, const char *const name) {
	const size_t name_len = strlen(name);
	struct YASL_String *string = YASL_String_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string);

	struct YASL_Object value = YASL_Table_search(&env->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END && env->parent == NULL) {
		printf("error in env_get with key: ");
		print(key);
		exit(EXIT_FAILURE);
	}
	if (value.type == Y_END) return env_get(env->parent, name);
	return value.value.ival;
}

int64_t env_decl_var(struct Env *const env, const char *const name) {
	const size_t name_len = strlen(name);
	struct YASL_String *string = YASL_String_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string);
	struct YASL_Object value = YASL_INT((long)env_len(env));
	YASL_Table_insert_fast(&env->vars, key, value);
	return env_len(env);
}

bool env_used_in_closure(const struct Env *const env) {
	if (env == NULL) return false;
	return env->used_in_closure || env_used_in_closure(env->parent);
}

static struct YASL_Table *get_closest_scope_with_var(struct Env *const env, const char *const name, const size_t name_len) {
	struct YASL_Object key = YASL_Table_search_string_int(&env->vars, name, name_len);
	return key.type != Y_END ? &env->vars : get_closest_scope_with_var(env->parent, name, name_len);
}

void env_make_const(struct Env *const env, const char *const name) {
	const size_t name_len = strlen(name);
	struct YASL_Table *ht = get_closest_scope_with_var(env, name, name_len);
	YASL_Table_insert_string_int(ht, name, name_len, ~YASL_Table_search_string_int(ht, name, name_len).value.ival);
}
