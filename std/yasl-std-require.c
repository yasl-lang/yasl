#include <yasl_state.h>
#include "yasl-std-require.h"
#include "yasl-std-math.h"
#include "yasl-std-io.h"
#include "yasl_plat.h"
#include "yasl_aux.h"

#define LOAD_LIB_FUN_NAME "YASL_load_dyn_lib"

struct YASL_State *YASL_newstate_num(char *filename, size_t num);

// TODO: rewrite this whole fucking mess. I'm not even sure if it works properly honestly.

int YASL_require(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		// TODO error message
		return YASL_TYPE_ERROR;
	}

	char *mode_str = YASL_peekcstr(S);
	YASL_pop(S);

	struct YASL_State *Ss = YASL_newstate_num(mode_str, S->vm.headers_size);

	if (!Ss) {
		// TODO error message
		return -1;
	}

	YASL_Table_del(Ss->compiler.strings);
	Ss->compiler.strings = S->compiler.strings;
	YASL_ByteBuffer_del(Ss->compiler.header);
	Ss->compiler.header = S->compiler.header;
	YASL_Table_del(Ss->vm.globals);
	Ss->vm.globals = S->vm.globals;
	// Ss->vm.constants = S->vm.constants;

	// Load Standard Libraries
	YASLX_decllibs(Ss);

	// Ss->vm.globals[Ss->vm.headers_size - 1] = Ss->vm.globals[0];
	// Ss->vm.globals[0] = NULL;
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
	S->vm.headers = (unsigned char **) realloc(S->vm.headers, new_headers_size * sizeof(unsigned char *));
	for (size_t i = old_headers_size; i < new_headers_size; i++) {
		S->vm.headers[i] = Ss->vm.headers[i];
		Ss->vm.headers[i] = NULL;
	}

	Ss->vm.globals = NULL;

	Ss->vm.code = NULL;
	S->vm.headers_size = S->vm.num_globals = new_headers_size;
	Ss->compiler.strings = NULL;
	Ss->compiler.header = YASL_ByteBuffer_new(0);
	for (int i = 0; i < S->vm.num_constants; i++) {
		dec_ref(S->vm.constants + i);
	}
	free(S->vm.constants);
	S->vm.constants = Ss->vm.constants;
	S->vm.num_constants = Ss->vm.num_constants;
	Ss->vm.constants = NULL;
	Ss->vm.num_constants = 0;

	YASL_delstate(Ss);

	vm_push(&S->vm, exported);
	dec_ref(&vm_peek(&S->vm));

	free(mode_str);

	return YASL_SUCCESS;
}

int YASL_require_c(struct YASL_State *S) {
	// TODO: Do I need anything else here?
	if (!YASL_isstr(S)) {
		vm_print_err_bad_arg_type((struct VM *) S, "require_c", 0, Y_STR, YASL_peektype(S));
		return YASL_TYPE_ERROR;
	}

	char *mode_str = YASL_peekcstr(S);
	YASL_pop(S);

#if defined(YASL_USE_WIN)
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
	void *lib = LoadLibrary(TEXT(mode_str));
	if (!lib) {
	    vm_print_err_value((struct VM *)S, "couldn't open shared library: %s.\n", mode_str);
	    return YASL_VALUE_ERROR;
	}
	int (*fun)(struct YASL_State *) =
		(int (*)(struct YASL_State *))GetProcAddress(lib, LOAD_LIB_FUN_NAME);
	if (!fun) {
	    vm_print_err_value((struct VM *)S, "couldn't load function: %s.\n", LOAD_LIB_FUN_NAME);
	    return YASL_VALUE_ERROR;
	}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
	return fun(S);
#elif defined(YASL_USE_UNIX) || defined(YASL_USE_APPLE)
	void *lib = dlopen(mode_str, RTLD_NOW);
	if (!lib) {
		vm_print_err_value((struct VM *) S, "couldn't open shared library: %s.\n", mode_str);
		return YASL_VALUE_ERROR;
	}
	int (*fun)(struct YASL_State *) = (int (*)(struct YASL_State *)) dlsym(lib, LOAD_LIB_FUN_NAME);
	if (!fun) {
		vm_print_err_value((struct VM *) S, "couldn't load function: %s.\n", LOAD_LIB_FUN_NAME);
		return YASL_VALUE_ERROR;
	}
	return fun(S);
#else
	(void) mode_str;
	return YASL_PLATFORM_NOT_SUPP;
#endif
}

int YASL_decllib_require(struct YASL_State *S) {
	YASL_declglobal(S, "require");
	YASL_pushcfunction(S, YASL_require, 1);
	YASL_setglobal(S, "require");

	return YASL_SUCCESS;
}

int YASL_decllib_require_c(struct YASL_State *S) {
	YASL_declglobal(S, "require_c");
	YASL_pushcfunction(S, YASL_require_c, 1);
	YASL_setglobal(S, "require_c");

	return YASL_SUCCESS;
}
