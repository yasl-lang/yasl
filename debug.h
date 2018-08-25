#pragma once

#include <color.h>
#include <stdio.h>

enum LogLevel {
    INFO,
    DEBUG,
    TRACE
};

#define LOGLEVEL DEBUG
#define YASL_INFO_LOG(str, msg) do { if (LOGLEVEL >= INFO) printf(K_RED str K_END, msg); } while(0)
#define YASL_DEBUG_LOG(str, msg) do { if (LOGLEVEL >= DEBUG) printf(K_YEL str K_END, msg); } while(0)
#define YASL_TRACE_LOG(str, msg) do { if (LOGLEVEL >= TRACE) printf(K_GRN str K_END, msg); } while(0)