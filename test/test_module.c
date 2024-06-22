#include "yasl.h"

static int hello_world(struct YASL_State *S) {
	YASL_pushlit(S, "hello world");
	return 1;
}

int YASL_load_dyn_lib(struct YASL_State *S) {
	YASL_pushcfunction(S, &hello_world, 0);
	return 1;
}
