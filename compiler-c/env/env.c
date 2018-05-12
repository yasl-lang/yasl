#include "env.h"

Env_t *env_new(Env_t *parent) {
    Env_t *env  = malloc(sizeof(Env_t));
    //env->parent = malloc(sizeof(Env_t));
    //env->vars   = malloc(sizeof(Hash_t));
    env->parent = parent;
    env->vars   = new_hash();
    return env;
}

void env_del(Env_t *env) {
    if (env->parent != NULL) env_del(env->parent);
    int i;
    for (i = 0; i < env->vars->size; i++) {
        Item_t* item = env->vars->items[i];
        if (item != NULL) {
            del_string8((String_t*)item->key->value);
            free(item->key);
            free(item->value);
            free(item);
        }
    }
    free(env->vars->items);
    free(env->vars);
    free(env->parent);
    free(env);

}

int64_t env_len(Env_t *env) {
    return env->vars->count + (env->parent == NULL ? 0 : env_len(env->parent));
}

int env_contains(Env_t *env, char *name, int64_t name_len) {
    String_t *string = malloc(sizeof(String_t));
    string->str = malloc(name_len);
    string->length = name_len;
    memcpy(string->str, name, string->length);
    Constant key = (Constant) { .value = (int64_t)string, .type = STR8 };

    Constant *value = ht_search(env->vars, key);
    if (value == NULL && env->parent == NULL) {
        return 0;
    }
    if (value == NULL) return env_get(env->parent, name, name_len);
    return 1;
}

int64_t env_get(Env_t *env, char *name, int64_t name_len) {
    String_t *string = malloc(sizeof(String_t));
    string->str = malloc(name_len);
    string->length = name_len;
    memcpy(string->str, name, string->length);
    Constant key = (Constant) { .value = (int64_t)string, .type = STR8 };

    Constant *value = ht_search(env->vars, key);
    if (value == NULL && env->parent == NULL) {
        printf("error in env_get with key: ");
        print(key);
        exit(EXIT_FAILURE);
    }
    if (value == NULL) return env_get(env->parent, name, name_len);
    return value->value;
}

void env_decl_var(Env_t *env, char *name, int64_t name_len) {
    String_t *string = malloc(sizeof(String_t));
    string->str = malloc(name_len);
    string->length = name_len;
    memcpy(string->str, name, string->length);
    Constant key = (Constant) { .value = (int64_t)string, .type = STR8 };
    Constant value = (Constant) { .value = env_len(env), .type = INT64 };
    ht_insert(env->vars, key, value);
}