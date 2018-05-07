//
// Created by thiabaud on 01/05/18.
//
#include "../../hashtable/hashtable.h" 
#include <string.h>

struct Env_s {
    struct Env_s *parent;
    Hash_t *vars;
};

typedef struct Env_s Env_t;

Env_t *env_new(Env_t *env);
void env_del(Env_t *env); // TODO: implement properly.

int64_t env_len(Env_t *env);
int env_contains(Env_t *env, char *name, int64_t name_len);
int64_t env_get(Env_t *env, char *name, int64_t name_len);
void env_decl_var(Env_t *env, char *name, int64_t name_len);

/*
class Env(object):
    def __init__(self, parent=None):
        self.vars = {}
        self.parent = parent
    def __len__(self):
        if self.parent is None:
            return len(self.vars)
        else:
            return len(self.vars) + len(self.parent)
    def __contains__(self, var:str):
        if var in self.vars: return True
        elif self.parent is None: return False
        else: return self.parent.__contains__(var)
    def __getitem__(self, var:str):
        if var in self.vars:
            return self.vars[var]
        elif self.parent is None:
            return None
        else:
            return self.parent.__getitem__(var)
    def __setitem__(self, key, value):
        self.vars[key] = value
    def decl_var(self, var:str ):
        self.vars[var] = len(self)
*/
