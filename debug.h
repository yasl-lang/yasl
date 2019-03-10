#pragma once

#include <stdio.h>

#include "yasl_include.h"

enum LogLevel {
	D_LEX      = 1,
	D_PARSE    = 2,
	D_FOLD     = 4,
	D_COMPILE  = 8,
	D_BYTECODE = 16,
	D_VM       = 32,
	D_RC       = 64
};

#define LOGLEVEL 0

#define YASL_LEX_DEBUG_LOG(str, msg) do { if ((LOGLEVEL) & D_LEX) printf(K_RED str K_END, msg); } while(0)
#define YASL_PARSE_DEBUG_LOG(str, msg) do { if ((LOGLEVEL) & D_PARSE) printf(K_YEL str K_END, msg); } while(0)
#define YASL_FOLD_DEBUG_LOG(str, msg) do { if ((LOGLEVEL) & D_FOLD) printf(K_WHT str K_END, msg); } while(0)
#define YASL_COMPILE_DEBUG_LOG(str, msg) do { if ((LOGLEVEL) & D_COMPILE) printf(K_GRN str K_END, msg); } while(0)
#define YASL_BYTECODE_DEBUG_LOG(str, msg) do { if ((LOGLEVEL) & D_BYTECODE) printf(K_BLU str K_END, msg); } while(0)
#define YASL_VM_DEBUG_LOG(str, msg) do { if ((LOGLEVEL) & D_VM) printf(K_CYN str K_END, msg); } while(0)
#define YASL_RC_DEBUG_LOG(str, msg) do { if ((LOGLEVEL) & D_RC) printf(K_MAG str K_END, msg); } while(0)