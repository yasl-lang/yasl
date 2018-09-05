#include <interpreter/YASL_string/YASL_string.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include "env.h"

Env_t *env_new(Env_t *parent) {
    Env_t *env  = malloc(sizeof(Env_t));
    env->parent = parent;
    env->vars   = ht_new();
    return env;
}

void env_del(Env_t *env) {
    if (env->parent != NULL) env_del(env->parent);
    free(env->parent);
    env_del_current_only(env);
}

void env_del_current_only(Env_t *env) {
    int i;
    for (i = 0; i < env->vars->size; i++) {
        Item_t* item = env->vars->items[i];
        if (item != NULL) {
            str_del(item->key->value.sval);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    free(env->vars->rc);
    free(env->vars->items);
    free(env->vars);
    free(env);
}

int64_t env_len(Env_t *env) {
    return env->vars->count + (env->parent == NULL ? 0 : env_len(env->parent));
}

int env_contains_cur_scope(Env_t *env, char *name, int64_t name_len) {
    String_t *string = str_new_sized(name_len, copy_char_buffer(name_len, name));
    YASL_Object key = (YASL_Object) { .value.sval = string, .type = Y_STR };

    YASL_Object *value = ht_search(env->vars, key);
    str_del(key.value.sval);
    if (value == NULL) {
        return 0;
    }
    return 1;
}

int env_contains(Env_t *env, char *name, int64_t name_len) {
    String_t *string = str_new_sized(name_len, copy_char_buffer(name_len, name));
    YASL_Object key = (YASL_Object) { .value.sval = string, .type = Y_STR };

    YASL_Object *value = ht_search(env->vars, key);
    str_del(key.value.sval);
    if (value == NULL && env->parent == NULL) {
        return 0;
    }
    if (value == NULL) return env_contains(env->parent, name, name_len);
    return 1;
}

int64_t env_get(Env_t *env, char *name, int64_t name_len) {
    String_t *string = str_new_sized(name_len, copy_char_buffer(name_len, name));
    YASL_Object key = (YASL_Object) { .value.sval = string, .type = Y_STR };

    YASL_Object *value = ht_search(env->vars, key);
    str_del(key.value.sval);
    if (value == NULL && env->parent == NULL) {
        printf("error in env_get with key: ");
        print(key);
        exit(EXIT_FAILURE);
    }
    if (value == NULL) return env_get(env->parent, name, name_len);
    return value->value.ival;
}

void env_decl_var(Env_t *env, char *name, int64_t name_len) {
    String_t *string = str_new_sized(name_len, copy_char_buffer(name_len, name));
    YASL_Object key = (YASL_Object) { .value.sval = string, .type = Y_STR };
    YASL_Object value = (YASL_Object) { .value.ival = env_len(env), .type = Y_INT64 };
    ht_insert(env->vars, key, value);
}

static Hash_t *get_closest_scope_with_var(Env_t *env, char *name, int64_t name_len) {
    YASL_Object *key = ht_search_string_int(env->vars, name, name_len);
    return key ? env->vars : get_closest_scope_with_var(env->parent, name, name_len);
}

void env_make_const(Env_t *env, char *name, int64_t name_len) {
    Hash_t *ht = get_closest_scope_with_var(env, name, name_len);
    ht_insert_string_int(ht, name, name_len, ~ht_search_string_int(ht, name, name_len)->value.ival);
}