#include "test/unit_tests/test_lexer/lexertest.h"
#include "test/unit_tests/test_compiler/compilertest.h"
#include "test/unit_tests/test_collections/collectiontest.h"
#include "test/unit_tests/test_methods/methodtest.h"

int main() {
	int failed = 0;
	failed = failed || lexertest();
	failed = failed || compilertest();
	failed = failed || collectiontest();
	failed = failed || methodtest();
	return failed;
}
