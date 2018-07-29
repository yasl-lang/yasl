#include "lexertest.h"
#include "compilertest.h"

int main() {
    int failed = 0;
    failed = failed || lexertest();
    failed = failed || compilertest();
    return failed;
}