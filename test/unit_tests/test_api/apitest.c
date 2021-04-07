#include "test/yats.h"
#include "pushtest.h"
#include "poptest.h"
#include "deltest.h"
#include "tablenexttest.h"

SETUP_YATS();

////////////////////////////////////////////////////////////////////////////////

int apitest() {
	RUN(pushtest);
	RUN(poptest);
	RUN(deltest);
	RUN(tablenexttest);
	return NUM_FAILED;
}
