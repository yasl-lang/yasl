#include "yasl.h"

#if defined(_MSC_VER)
#define YASL_MODULE_EXPORT __declspec(dllexport)
#else
#define YASL_MODULE_EXPORT
#endif

static int hello_world(struct YASL_State *S) {
	YASL_pushlit(S, "hello world");
	return 1;
}

#ifdef __cplusplus
extern "C" {
#endif

int YASL_MODULE_EXPORT YASL_load_dyn_lib(struct YASL_State *S) {
	YASL_pushcfunction(S, &hello_world, 0);
	return 1;
}

#ifdef __cplusplus
};
#endif

