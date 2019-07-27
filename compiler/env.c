#include "env.h"

#include <stdio.h>

#include "data-structures/YASL_string.h"
#include "interpreter/YASL_Object.h"

Env_t *env_new(Env_t *parent) {
	Env_t *env = (Env_t *)malloc(sizeof(Env_t));
	env->parent = parent;
	env->vars = table_new();
	return env;
}

void env_del(Env_t *env) {
	if (env == NULL) return;
	env_del(env->parent);
	free(env->parent);
	env_del_current_only(env);
}

void env_del_current_only(Env_t *env) {
	for (size_t i = 0; i < env->vars->size; i++) {
		struct YASL_HashTable_Item *item = &env->vars->items[i];
		if (item->key.type != Y_END && item->key.type != Y_UNDEF) {
			str_del(item->key.value.sval);
			// free(item);
		}
	}
	free(env->vars->items);
	free(env->vars);
	free(env);
}

size_t env_len(const Env_t *const env) {
	if (env == NULL) return 0;
	return env->vars->count + env_len(env->parent);
}

int env_contains_cur_scope(const Env_t *const env, const char *const name, const size_t name_len) {
	struct YASL_String *string = str_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string); // (struct YASL_Object) { .value.sval = string, .type = Y_STR };

	struct YASL_Object value = table_search(env->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END) {
		return 0;
	}
	return 1;
}

int env_contains(const Env_t *const env, const char *const name, const size_t name_len) {
	if (env == NULL) return 0;
	struct YASL_String *string = str_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string);

	struct YASL_Object value = table_search(env->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END && env->parent == NULL) {
		return 0;
	}
	if (value.type == Y_END) return env_contains(env->parent, name, name_len);
	return 1;
}

int64_t env_get(const Env_t *const env, const char *const name, const size_t name_len) {
	struct YASL_String *string = str_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string);

	struct YASL_Object value = table_search(env->vars, key);
	str_del(key.value.sval);
	if (value.type == Y_END && env->parent == NULL) {
		printf("error in env_get with key: ");
		print(key);
		exit(EXIT_FAILURE);
	}
	if (value.type == Y_END) return env_get(env->parent, name, name_len);
	return value.value.ival;
}

int64_t env_decl_var(Env_t *const env, const char *const name, const size_t name_len) {
	struct YASL_String *string = str_new_sized_heap(0, name_len, copy_char_buffer(name_len, name));
	struct YASL_Object key = YASL_STR(string);
	struct YASL_Object value = YASL_INT((long)env_len(env));
	table_insert(env->vars, key, value);
	return env_len(env);
}

static struct YASL_HashTable *get_closest_scope_with_var(const Env_t *const env, const char *const name, const size_t name_len) {
	struct YASL_Object key = table_search_string_int(env->vars, name, name_len);
	return key.type != Y_END ? env->vars : get_closest_scope_with_var(env->parent, name, name_len);
}

void env_make_const(Env_t *const env, const char *const name, const size_t name_len) {
	struct YASL_HashTable *ht = get_closest_scope_with_var(env, name, name_len);
	table_insert_string_int(ht, name, name_len, ~table_search_string_int(ht, name, name_len).value.ival);
}
