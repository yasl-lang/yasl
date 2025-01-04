#include "test/unit_tests/test_lexer/lexertest.h"
#include "test/unit_tests/test_compiler/compilertest.h"
#include "test/unit_tests/test_collections/collectiontest.h"
#include "test/unit_tests/test_methods/methodtest.h"
#include "test/unit_tests/test_api/apitest.h"
#include "test/unit_tests/test_vm/vmtest.h"
#include "test/unit_tests/test_gc/gctest.h"
#include "test/unit_tests/test_env/envtest.h"
#include "test/unit_tests/test_util/utiltest.h"

int unit_tests(void) {
	int failed = 0;
	failed += apitest();
	failed += lexertest();
	failed += compilertest();
	failed += collectiontest();
	failed += methodtest();
	failed += vmtest();
	failed += gctest();
	failed += envtest();
	failed += utiltest();
	return failed;
}
