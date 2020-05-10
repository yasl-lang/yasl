#include "yasl_conf.h"

/*
 * This file is for including platform-specifc headers, depending on what platform we are using.
 */

#if defined(YASL_USE_UNIX)
#include <dlfcn.h>
#elif defined(YASL_USE_WIN)

#elif defined(YASL_USE_APPLE)

#else
/* Unknown Platform */
#endif
