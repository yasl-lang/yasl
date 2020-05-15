#include "yasl-std-math.h"
#include "yasl-std-io.h"
#include "yasl-std-require.h"
#include "yasl-std-collections.h"

static int load_libs(struct YASL_State *S) {
	YASL_load_math(S);
	YASL_load_io(S);
	YASL_load_require(S);
	YASL_load_collections(S);
	return YASL_SUCCESS;
}
