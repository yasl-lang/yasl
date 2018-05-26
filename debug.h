#pragma once

#include <stdio.h>

#define DEBUG 0
#define YASL_DEBUG_LOG(str, msg) { if (DEBUG) printf(str, msg); }
