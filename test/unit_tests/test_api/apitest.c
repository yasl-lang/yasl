#include "test/yats.h"
#include "pushtest.h"
#include "poptest.h"
#include "deltest.h"
#include "fntest.h"

SETUP_YATS();

////////////////////////////////////////////////////////////////////////////////

int apitest() {
	RUN(deltest);
	RUN(fntest);
	RUN(poptest);
	RUN(pushtest);
	return NUM_FAILED;
}
