#include <yasl_state.h>
#include <bytebuffer/bytebuffer.h>
#include "yasl-std-require.h"
#include "yasl-std-math.h"
#include "yasl-std-io.h"


int YASL_require(struct YASL_State *S) {
	struct YASL_Object *mode = YASL_popobject(S);
	char *mode_str;

	if (YASL_isstring(mode) == YASL_SUCCESS) {
		mode_str = YASL_getcstring(mode);
	} else {
		return -1;
	}

	struct YASL_State *Ss = YASL_newstate(mode_str);

	if (!Ss) {
		return -1;
	}

	// Load Standard Libraries
	YASL_load_math(Ss);
	YASL_load_io(Ss);
	YASL_load_require(Ss);

	int status = YASL_execute(Ss);

	if (status != YASL_MODULE_SUCCESS) {
		puts("Not a valid module");
		return YASL_ERROR;
	}
	inc_ref(&vm_peek(&Ss->vm));
	struct YASL_Object exported = vm_pop(&Ss->vm);

	vm_pushundef(&Ss->vm);

	size_t old_headers_size = S->vm.headers_size;
	S->vm.headers_size += 1 + Ss->vm.headers_size;
	S->vm.headers = (unsigned char **)realloc(S->vm.headers, sizeof(unsigned char *) * S->vm.headers_size);
	S->vm.headers[old_headers_size++] = Ss->vm.code;
	Ss->vm.code = NULL;
	for (size_t i = 0; i < Ss->vm.headers_size; i++) {
		S->vm.headers[i + old_headers_size] = Ss->vm.headers[i];
		Ss->vm.headers[i] = NULL;
	}

	YASL_delstate(Ss);

	vm_push(&S->vm, exported);
	dec_ref(&vm_peek(&S->vm));

	free(mode_str);
	// printf("rc: %zd\n", table->rc->refs);
	return status == YASL_MODULE_SUCCESS ? YASL_SUCCESS : YASL_ERROR;

}

int YASL_load_require(struct YASL_State *S) {
	struct YASL_Object *require = YASL_CFunction(YASL_require, 1);

	YASL_declglobal(S, "require");
	YASL_pushobject(S, require);
	YASL_setglobal(S, "require");

	// free(require);

	return YASL_SUCCESS;
}
