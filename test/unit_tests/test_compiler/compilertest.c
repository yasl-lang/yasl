#include "src/opcode.h"
#include "yats.h"
#include "compilertest.h"
#include "binoptest.h"
#include "closuretest.h"
#include "unoptest.h"
#include "literaltest.h"
#include "iftest.h"
#include "whiletest.h"
#include "fortest.h"
#include "foreachtest.h"
#include "functiontest.h"
#include "comprehensiontest.h"
#include "foldingtest.h"
#include "syntaxerrortest.h"
#include "matchtest.h"

SETUP_YATS();

// NOTE: these tests depend on the endianess of the system, so they may fail on big endian systems.

////////////////////////////////////////////////////////////////////////////////

int compilertest() {
	RUN(literaltest);
	RUN(unoptest);
	RUN(binoptest);
	RUN(closuretest);
	RUN(iftest);
	RUN(whiletest);
	RUN(fortest);
	RUN(foreachtest);
	RUN(functiontest);
	RUN(comprehensiontest);
	RUN(foldingtest);
	RUN(syntaxerrortest);
	RUN(matchtest);

	return NUM_FAILED;
}
