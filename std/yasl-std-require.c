#include <yasl_state.h>
#include <data-structures/YASL_ByteBuffer.h>
#include "yasl-std-require.h"
#include "yasl-std-math.h"
#include "yasl-std-io.h"
#include "yasl_plat.h"

#define LOAD_LIB_FUN_NAME "YASL_load_dyn_lib"

struct YASL_State *YASL_newstate_num(char *filename, size_t num);

// TODO: rewrite this whole fucking mess. I'm not even sure if it works properly honestly.

int YASL_require(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		// TODO error message
		return YASL_TYPE_ERROR;
	}

	char *mode_str = YASL_top_peekcstring(S);
	YASL_pop(S);

	struct YASL_State *Ss = YASL_newstate_num(mode_str, S->vm.headers_size);

	if (!Ss) {
		// TODO error message
		return -1;
	}

	// Load Standard Libraries
	YASL_load_math(Ss);
	YASL_load_io(Ss);
	YASL_load_require(Ss);

	Ss->vm.globals[Ss->vm.headers_size - 1] = Ss->vm.globals[0];
	Ss->vm.globals[0] = NULL;
	int status = YASL_execute(Ss);

	if (status != YASL_MODULE_SUCCESS) {
		puts("Not a valid module");
		return YASL_ERROR;
	}
	inc_ref(&vm_peek(&Ss->vm));
	struct YASL_Object exported = vm_pop(&Ss->vm);

	vm_pushundef(&Ss->vm);

	size_t old_headers_size = S->vm.headers_size;
	size_t new_headers_size = Ss->vm.headers_size;
	S->vm.headers = (unsigned char **)realloc(S->vm.headers, new_headers_size * sizeof(unsigned char *));
	S->vm.globals = (struct YASL_Table **)realloc(S->vm.globals, new_headers_size * sizeof(struct YASL_Table *));
	for (size_t i = old_headers_size; i < new_headers_size; i++) {
		S->vm.headers[i] = Ss->vm.headers[i];
		Ss->vm.headers[i] = NULL;
		S->vm.globals[i] = Ss->vm.globals[i];
		Ss->vm.globals[i] = NULL;
	}
	Ss->vm.code = NULL;
	S->vm.headers_size = S->vm.num_globals = new_headers_size;
	YASL_delstate(Ss);

	vm_push(&S->vm, exported);
	dec_ref(&vm_peek(&S->vm));

	free(mode_str);

	return YASL_SUCCESS;
}

int YASL_require_c(struct YASL_State *S) {
	if (!YASL_top_isstring(S)) {
		vm_print_err_bad_arg_type((struct VM *)S, "require_c", 0, Y_STR, YASL_top_peektype(S));
		return YASL_TYPE_ERROR;
	}

	char *mode_str = YASL_top_peekcstring(S);
	YASL_pop(S);

#if defined(YASL_USE_WIN)
	return YASL_PLATFORM_NOT_SUPP;
#elif defined(YASL_USE_UNIX)
	void *lib = dlopen(mode_str, RTLD_NOW);
	if (!lib) puts(dlerror());
	int (*fun)(struct YASL_State *) = (int (*)(struct YASL_State *))dlsym(lib, LOAD_LIB_FUN_NAME);
	if (!fun) puts(dlerror());
	return fun(S);
#else
	return YASL_PLATFORM_NOT_SUPP;
#endif
}

int YASL_load_require(struct YASL_State *S) {
	YASL_declglobal(S, "require");
	YASL_pushcfunction(S, YASL_require, 1);
	YASL_setglobal(S, "require");

	YASL_declglobal(S, "require_c");
	YASL_pushcfunction(S, YASL_require_c, 1);
	YASL_setglobal(S, "require_c");

	return YASL_SUCCESS;
}
