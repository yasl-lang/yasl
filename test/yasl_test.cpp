#include "test/test_lexer/lexertest.h"
#include "test/test_compiler/compilertest.h"

int main() {
    int failed = 0;
    failed = failed || lexertest();
    failed = failed || compilertest();
    return failed;
}
