#include <yasl_state.h>
#include "yasl_plat.h"
#include "yasl_aux.h"

#define LOAD_LIB_FUN_NAME "YASL_load_dyn_lib"

struct YASL_State *YASL_newstate_num(const char *filename, size_t num);

// TODO: rewrite this whole fucking mess. I'm not even sure if it works properly honestly.

static struct YASL_State *open_on_path(const char *path, const char *name, const char sep, const char dirmark, const size_t num) {
	const char *start = path;
	const char *end = strchr(start, dirmark);
	while (end != NULL) {
		char *buffer = (char *)malloc(end - start + strlen(name) + 1);
		const char *split = strchr(start, sep);
		memcpy(buffer, start, split - start);
		memcpy(buffer + (split - start), name, strlen(name));
		memcpy(buffer + (split - start) + strlen(name), split + 1, end - split - 1);
		buffer[end - start + strlen(name) - 1] = '\0';

		struct YASL_State *S = YASL_newstate_num(buffer, num);
		free(buffer);
		if (S)
			return S;

		start = end + 1;
		end = strchr(start, dirmark);
	}

	return YASL_newstate_num(name, num);
}

int YASL_require(struct YASL_State *S) {
	if (!YASL_isstr(S)) {
		// TODO error message
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	char *mode_str = YASL_peekcstr(S);
	YASL_pop(S);

	struct YASL_State *Ss = open_on_path(YASL_DEFAULT_PATH, mode_str, YASL_PATH_MARK, YASL_PATH_SEP, S->vm.headers_size);

	if (!Ss) {
		YASL_print_err(S, "could not open package %s.", mode_str);
		free(mode_str);
		YASL_throw_err(S, YASL_ERROR);
	}

	YASL_Table_del(Ss->compiler.strings);
	Ss->compiler.strings = S->compiler.strings;
	YASL_ByteBuffer_del(Ss->compiler.header);
	Ss->compiler.header = S->compiler.header;
	YASL_Table_del(Ss->vm.globals);
	Ss->vm.globals = S->vm.globals;

	// Load Standard Libraries
	YASLX_decllibs(Ss);

	YASL_Table_del(Ss->vm.metatables);
	Ss->vm.metatables = S->vm.metatables;

	int status = YASL_execute(Ss);

	if (status != YASL_MODULE_SUCCESS) {
		puts("Not a valid module");
		YASL_throw_err(S, YASL_ERROR);
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
	S->vm.headers_size = new_headers_size;

	Ss->vm.globals = NULL;
	Ss->vm.metatables = NULL;

	Ss->vm.code = NULL;
	Ss->compiler.strings = NULL;
	Ss->compiler.header = YASL_ByteBuffer_new(0);
	for (int i = 0; i < S->vm.num_constants; i++) {
		vm_dec_ref(&S->vm, S->vm.constants + i);
	}
	free(S->vm.constants);
	S->vm.constants = Ss->vm.constants;
	S->vm.num_constants = Ss->vm.num_constants;
	Ss->vm.constants = NULL;
	Ss->vm.num_constants = 0;

	YASL_delstate(Ss);

	vm_push(&S->vm, exported);
	vm_dec_ref(&S->vm, &vm_peek(&S->vm));

	free(mode_str);
	return 1;
}

static void *search_on_path(const char *path, const char *name, const char sep, const char dirmark) {
	const char *start = path;
	const char *end = strchr(start, dirmark);
	while (end != NULL) {
		char *buffer = (char *)malloc(end - start + strlen(name) + 1);
		const char *split = strchr(start, sep);
		memcpy(buffer, start, split - start);
		memcpy(buffer + (split - start), name, strlen(name));
		memcpy(buffer + (split - start) + strlen(name), split + 1, end - split - 1);
		buffer[end - start + strlen(name) - 1] = '\0';

#if defined(YASL_USE_WIN)
		void *lib = LoadLibrary(TEXT(buffer));
#elif defined(YASL_USE_UNIX) || defined(YASL_USE_APPLE)
		void *lib = dlopen(buffer, RTLD_NOW);
#else
		void *lib = NULL;
#endif
		free(buffer);
		if (lib)
			return lib;

		start = end + 1;
		end = strchr(start, dirmark);
	}
#if defined(YASL_USE_WIN)
	return LoadLibrary(TEXT(path));
#elif defined(YASL_USE_UNIX) || defined(YASL_USE_APPLE)
	return dlopen(name, RTLD_NOW);
#else
	return NULL;
#endif
}

int YASL_require_c(struct YASL_State *S) {
	// TODO: Do I need anything else here?
	if (!YASL_isstr(S)) {
		YASLX_print_err_bad_arg_type(S, "__require_c__", 0, YASL_STR_NAME, YASL_peekntypename(S, 0));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	char *path = YASL_peekcstr(S);
	YASL_pop(S);

#if defined(YASL_USE_WIN)
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
	void *lib = search_on_path(YASL_DEFAULT_CPATH, path, YASL_PATH_MARK, YASL_PATH_SEP);
	if (!lib) {
	    vm_print_err_value((struct VM *)S, "couldn't open shared library: %s.\n", path);
	    YASL_throw_err(S, YASL_VALUE_ERROR);
	}
	YASL_cfn fun = (YASL_cfn)GetProcAddress(lib, LOAD_LIB_FUN_NAME);
	if (!fun) {
	    vm_print_err_value((struct VM *)S, "couldn't load function: %s.\n", LOAD_LIB_FUN_NAME);
	    YASL_throw_err(S, YASL_VALUE_ERROR);
	}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
	fun(S);
#elif defined(YASL_USE_UNIX) || defined(YASL_USE_APPLE)
	void *lib = search_on_path(YASL_DEFAULT_CPATH, path, YASL_PATH_MARK, YASL_PATH_SEP);
	free(path);
	if (!lib) {
		vm_print_err_value((struct VM *) S, "%s\n", dlerror());
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}
	YASL_cfn fun= (YASL_cfn) dlsym(lib, LOAD_LIB_FUN_NAME);
	if (!fun) {
		vm_print_err_value((struct VM *) S, "%s\n", dlerror());
		YASL_throw_err(S, YASL_VALUE_ERROR);
	}
	fun(S);
#else
	(void) path;
	YASL_throw_err(S, YASL_PLATFORM_NOT_SUPP);
#endif
	return 1;
}

int YASL_decllib_require(struct YASL_State *S) {
	YASL_declglobal(S, "require");
	YASL_pushcfunction(S, YASL_require, 1);
	YASL_setglobal(S, "require");

	return YASL_SUCCESS;
}

int YASL_decllib_require_c(struct YASL_State *S) {
	YASL_declglobal(S, "__require_c__");
	YASL_pushcfunction(S, YASL_require_c, 1);
	YASL_setglobal(S, "__require_c__");

	return YASL_SUCCESS;
}
