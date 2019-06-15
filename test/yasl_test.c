#include "test/test_lexer/lexertest.h"
#include "test/test_compiler/compilertest.h"
#include "test/test_collections/collectiontest.h"

int main() {
	int failed = 0;
	failed = failed || lexertest();
	failed = failed || compilertest();
	failed = failed || collectiontest();
	return failed;
}
