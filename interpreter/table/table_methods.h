#pragma once

struct YASL_State;

int table___get(struct YASL_State *S);

int table___set(struct YASL_State *S);

int table_keys(struct YASL_State *S);

int table_values(struct YASL_State *S);

int table_clone(struct YASL_State *S);