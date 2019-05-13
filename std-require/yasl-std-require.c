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
		// puts("ERROR: cannot open file.");
		//exit(EXIT_FAILURE);
		// YASL_pushundef(S);
		return -1;
	}

	// Load Standard Libraries
	YASL_load_math(Ss);
	YASL_load_io(Ss);
	YASL_load_require(Ss);

	int64_t old_len = *((int64_t *)(S->vm.code + 8));
	// printf("%ld\n", old_len);
	Ss->compiler.header->count = (size_t)old_len;
	int status = YASL_execute(Ss);

	struct RC_UserData *table = Ss->vm.global_vars;
	// printf("rc: %zd\n", table->rc->refs);
	Ss->vm.global_vars = NULL;
	ByteBuffer *bb = Ss->compiler.code;
	unsigned char *code = Ss->vm.code;
	// printf("%zd\n", Ss->compiler.code->count + Ss->compiler.header->count + 1);

	S->vm.code = (unsigned char *)realloc(S->vm.code, old_len + Ss->compiler.header->count);
	*((int64_t *)(&S->vm.code + 8)) += Ss->compiler.header->count;

	memcpy(S->vm.code + old_len, Ss->compiler.header + old_len, Ss->compiler.header->count - old_len);
	//*/
	YASL_delstate(Ss);

	vm_push(&S->vm, YASL_TABLE(table));

	free(mode_str);
	// printf("rc: %zd\n", table->rc->refs);
	return status;

}

int YASL_load_require(struct YASL_State *S) {
	struct YASL_Object *require = YASL_CFunction(YASL_require, 1);

	YASL_declglobal(S, "require");
	YASL_pushobject(S, require);
	YASL_setglobal(S, "require");

	// free(require);

	return YASL_SUCCESS;
}
